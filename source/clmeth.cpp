/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"


PyMethodDef pyext::meth_tbl[] = 
{
    {"__init__", pyext::pyext__init__, METH_VARARGS, "Constructor"},
    {"__del__", pyext::pyext__del__, METH_VARARGS, "Destructor"},

    {"_outlet", pyext::pyext_outlet, METH_VARARGS,"Send message to outlet"},
	{"_tocanvas", pyext::pyext_tocanvas, METH_VARARGS,"Send message to canvas" },

	{ "_bind", pyext::pyext_bind, METH_VARARGS,"Bind function to a receiving symbol" },
	{ "_unbind", pyext::pyext_unbind, METH_VARARGS,"Unbind function from a receiving symbol" },
#ifdef FLEXT_THREADS
	{ "_detach", pyext::pyext_detach, METH_VARARGS,"Set detach flag for called methods" },
#endif
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMethodDef pyext::attr_tbl[] =
{
	{ "__setattr__", pyext::pyext_setattr, METH_VARARGS,"Set class attribute" },
	{ "__getattr__", pyext::pyext_getattr, METH_VARARGS,"Get class attribute" },
	{ NULL, NULL,0,NULL },
};


const C *pyext::pyext_doc =
	"py/pyext - python external object for PD and MaxMSP, (C)2002 Thomas Grill\n"
	"\n"
	"This is the pyext base class. Available methods:\n"
	"_outlet(self,ix,args...): Send a message to an indexed outlet\n"
	"_tocanvas(self,args...): Send a message to the parent canvas\n"
#ifdef FLEXT_THREADS
	"_detach(self,int): Define whether called Python method run in their own threads\n"
#endif
	"_bind(self,name,func): Bind a python function to a symbol\n"
	"_unbind(self,name,func): Unbind a python function from a symbol\n"
;

PyObject* pyext::pyext__init__(PyObject *,PyObject *args)
{
//    post("pyext.__init__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::pyext__del__(PyObject *,PyObject *args)
{
//    post("pyext.__del__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::pyext_setattr(PyObject *,PyObject *args)
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

PyObject* pyext::pyext_getattr(PyObject *,PyObject *args)
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

PyObject *pyext::pyext_outlet(PyObject *,PyObject *args) 
{
	PyObject *self;
	AtomList *rargs = GetPyArgs(args,&self);

//    post("pyext.outlet called, args:%i",rargc);

	if(rargs) {
		pyext *ext = GetThis(self);
		if(!ext) 
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);

		if(ext && rargs && rargs->Count() >= 2) {
			I o = GetAInt((*rargs)[1]);
			if(o >= 1 && o <= ext->Outlets()) {
				if(rargs->Count() >= 3 && IsSymbol((*rargs)[2]))
					ext->ToOutAnything(o-1,GetSymbol((*rargs)[2]),rargs->Count()-3,rargs->Atoms()+3);
				else
					ext->ToOutList(o-1,rargs->Count()-2,rargs->Atoms()+2);
			}
			else
				post("pyext: outlet index out of range");
		}
		else {
//			PyErr_Print();
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
		}
	}

	if(rargs) delete rargs;

    Py_INCREF(Py_None);

    return Py_None;
}

#ifdef FLEXT_THREADS
//! Detach threads
PyObject *pyext::pyext_detach(PyObject *,PyObject *args)
{
	PyObject *self; 
	int val;
    if(!PyArg_ParseTuple(args, "Oi:py_detach",&self,&val)) {
        // handle error
		post("pyext - Syntax: _detach(self,[0/1])");
    }
	else {
		pyext *ext = GetThis(self);
		ext->m_detach(val != 0);
	}

    Py_INCREF(Py_None);
    return Py_None;
}
#endif

//! Send message to canvas
PyObject *pyext::pyext_tocanvas(PyObject *,PyObject *args)
{
	BL ok = false;
    if(PyTuple_Check(args)) {
		PyObject *self = PyTuple_GetItem(args,0);
		if(self && PyInstance_Check(self)) {
			pyext *ext = GetThis(self);

#ifdef PD
			PyObject *val = PyTuple_GetSlice(args,1,PyTuple_Size(args));
			AtomList *lst = GetPyArgs(val);
			if(lst) {
				t_glist *gl = ext->thisCanvas(); //canvas_getcurrent();
			    t_class **cl = (t_pd *)gl;
				if(cl) 
					pd_forwardmess(cl,lst->Count(),lst->Atoms());
#ifdef _DEBUG
				else
					post("pyext - no parent canvas?!");
#endif
				ok = true;
			}
			else 
				post("py/pyext - No data to send");
			if(lst) delete lst;
			Py_DECREF(val);
#else
#pragma message ("Not implemented for MaxMSP")
#endif
		}
	}

	if(!ok)	post("pyext - Syntax: _tocanvas(self,args...)");

    Py_INCREF(Py_None);
    return Py_None;
}
