#include "pyext.h"

PyObject* pyext::py__init__(PyObject *,PyObject *args)
{
//    post("pyext.__init__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::py__del__(PyObject *,PyObject *args)
{
//    post("pyext.__del__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::py_setattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*val,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OOO:test_foo", &self,&name,&val)) {
        // handle error
		error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
		return NULL;
    }

	BL handled = false;
    if(PyString_Check(name)) {
	    char* sname = PyString_AsString(name);
		if (sname) {
//			post("pyext::setattr %s",sname);
		}
	}

	if(!handled) {
		if(PyInstance_Check(self)) 
			PyDict_SetItem(((PyInstanceObject *)self)->in_dict, name,val);
		else
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* pyext::py_getattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OO:test_foo", &self,&name)) {
        // handle error
		error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }

    if(PyString_Check(name)) {
	    char* sname = PyString_AsString(name);
		if (sname) {
			if(!strcmp(sname,"_shouldexit")) {
				pyext *ext = GetThis(self); 
				if(ext)
					ret = PyLong_FromLong(ext->shouldexit?1:0);
			}
//			post("pyext::getattr %s",sname);
		}
	}

	return ret?ret:PyObject_GenericGetAttr(self,name);
}

PyObject *pyext::py_outlet(PyObject *,PyObject *args) 
{
//	PY_LOCK

	I rargc;
	PyObject *self;
	t_atom *rargv = GetPyArgs(rargc,args,&self);

//    post("pyext.outlet called, args:%i",rargc);

	if(rargv) {
		pyext *ext = GetThis(self);
		if(!ext) 
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);

		if(ext && rargv && rargc >= 2) {
			I o = GetAInt(rargv[1]);
			if(o >= 1 && o <= ext->Outlets()) {
				if(rargc >= 3 && IsSymbol(rargv[2]))
					ext->ToOutAnything(o-1,GetSymbol(rargv[2]),rargc-3,rargv+3);
				else
					ext->ToOutList(o-1,rargc-2,rargv+2);
			}
			else
				post("pyext: outlet index out of range");
		}
		else {
//			PyErr_Print();
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
		}
	}

	if(rargv) delete[] rargv;

    Py_INCREF(Py_None);

//	PY_UNLOCK
    return Py_None;
}


PyMethodDef pyext::meth_tbl[] = 
{
    {"__init__", pyext::py__init__, METH_VARARGS, "pyext __init__"},
    {"__del__", pyext::py__del__, METH_VARARGS, "pyext __del__"},
    {"_outlet", pyext::py_outlet, METH_VARARGS,"pyext outlet"},
	{ "_bind", pyext::py_bind, METH_VARARGS,"Bind function to a receiving symbol" },
	{ "_unbind", pyext::py_unbind, METH_VARARGS,"Unbind function from a receiving symbol" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMethodDef pyext::attr_tbl[] =
{
	{ "__setattr__", pyext::py_setattr, METH_VARARGS,"pyext setattr" },
	{ "__getattr__", pyext::py_getattr, METH_VARARGS,"pyext getattr" },
	{ NULL, NULL,0,NULL },
};

