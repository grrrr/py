/* 

py/pyext - python external object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"


PyMethodDef pyext::meth_tbl[] = 
{
/*
    {"__init__", pyext::pyext__init__, METH_VARARGS, "Constructor"},
    {"__del__", pyext::pyext__del__, METH_VARARGS, "Destructor"},
*/
    {"_outlet", pyext::pyext_outlet, METH_VARARGS,"Send message to outlet"},
#if FLEXT_SYS == FLEXT_SYS_PD
	{"_tocanvas", pyext::pyext_tocanvas, METH_VARARGS,"Send message to canvas" },
#endif

	{ "_bind", pyext::pyext_bind, METH_VARARGS,"Bind function to a receiving symbol" },
	{ "_unbind", pyext::pyext_unbind, METH_VARARGS,"Unbind function from a receiving symbol" },
#ifdef FLEXT_THREADS
	{ "_detach", pyext::pyext_detach, METH_VARARGS,"Set detach flag for called methods" },
	{ "_stop", pyext::pyext_stop, METH_VARARGS,"Stop running threads" },
#endif
	{ "_isthreaded", pyext::pyext_isthreaded, METH_O,"Query whether threading is enabled" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMethodDef pyext::attr_tbl[] =
{
	{ "__setattr__", pyext::pyext_setattr, METH_VARARGS,"Set class attribute" },
	{ "__getattr__", pyext::pyext_getattr, METH_VARARGS,"Get class attribute" },
	{ NULL, NULL,0,NULL },
};


const char *pyext::pyext_doc =
	"py/pyext - python external object for PD and Max/MSP, (C)2002-2005 Thomas Grill\n"
	"\n"
	"This is the pyext base class. Available methods:\n"
	"_outlet(self,ix,args...): Send a message to an indexed outlet\n"
#if FLEXT_SYS == FLEXT_SYS_PD
	"_tocanvas(self,args...): Send a message to the parent canvas\n"
#endif
	"_bind(self,name,func): Bind a python function to a symbol\n"
	"_unbind(self,name,func): Unbind a python function from a symbol\n"
#ifdef FLEXT_THREADS
	"_detach(self,int): Define whether a called Python method has its own thread\n"
	"_stop(self): Stop running threads\n"
#endif
	"_isthreaded(self): Query whether threading is enabled\n"
;

/*
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
*/

PyObject* pyext::pyext_setattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*val,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OOO:test_foo", &self,&name,&val)) {
        // handle error
		ERRINTERNAL();
		return NULL;
    }

	bool handled = false;

/*
    if(PyString_Check(name)) {
	    char* sname = PyString_AsString(name);
		if (sname) {
//			post("pyext::setattr %s",sname);
		}
	}
*/
	if(!handled) {
		if(PyInstance_Check(self)) 
			PyDict_SetItem(((PyInstanceObject *)self)->in_dict, name,val);
		else
			ERRINTERNAL();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* pyext::pyext_getattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OO:test_foo", &self,&name)) {
        // handle error
		ERRINTERNAL();
    }

    if(PyString_Check(name)) {
	    char* sname = PyString_AS_STRING(name);
		if(sname) {
			if(!strcmp(sname,"_shouldexit")) {
				pyext *ext = GetThis(self); 
				if(ext)
					ret = PyLong_FromLong(ext->shouldexit?1:0);
			}
//			post("pyext::getattr %s",sname);
		}
	}

	if(!ret) { 
#if PY_VERSION_HEX >= 0x02020000
		ret = PyObject_GenericGetAttr(self,name); // new reference (?)
#else
		if(PyInstance_Check(self))
            // borrowed reference
			ret = PyDict_GetItem(((PyInstanceObject *)self)->in_dict,name);	
#endif
	}
	return ret;
}

//! Send message to outlet
PyObject *pyext::pyext_outlet(PyObject *,PyObject *args)
{
	bool ok = false;

    // should always be a tuple!
    FLEXT_ASSERT(PyTuple_Check(args));

    int sz = PyTuple_GET_SIZE(args);

    // borrowed references!
    PyObject *self,*outl;

	if(
        sz >= 2 &&
		(self = PyTuple_GET_ITEM(args,0)) != NULL && PyInstance_Check(self) && 
		(outl = PyTuple_GET_ITEM(args,1)) != NULL && PyInt_Check(outl)
	) {
		pyext *ext = GetThis(self);

		PyObject *val;
        
        bool tp = 
            sz == 3 && 
            PySequence_Check(
                val = PyTuple_GET_ITEM(args,2) // borrow reference
            );

		if(!tp)
			val = PySequence_GetSlice(args,2,sz);  // new ref

		AtomList *lst = GetPyArgs(val);
		if(lst) {
			int o = PyInt_AsLong(outl);
			if(o >= 1 && o <= ext->Outlets()) {
				// by using the queue there is no immediate call of the next object
				// deadlock would occur if this was another py/pyext object!
				if(lst->Count() && IsSymbol((*lst)[0]))
					ext->ToQueueAnything(o-1,GetSymbol((*lst)[0]),lst->Count()-1,lst->Atoms()+1);
				else if(lst->Count() > 1)
					ext->ToQueueList(o-1,*lst);
                else
                    ext->ToQueueAtom(o-1,*lst->Atoms());
			}
			else
				post("pyext: outlet index out of range");

			ok = true;
		}
		else 
			post("py/pyext - No data to send");
		if(lst) delete lst;

		if(!tp) Py_DECREF(val);
	}

	if(!ok)	post("pyext - Syntax: _outlet(self,outlet,args...)");

    Py_INCREF(Py_None);
    return Py_None;
}



#ifdef FLEXT_THREADS
//! Detach threads
PyObject *pyext::pyext_detach(PyObject *,PyObject *args)
{
	PyObject *self; 
	int val;
    if(!PyArg_ParseTuple(args, "Oi:pyext_detach",&self,&val)) {
        // handle error
		post("pyext - Syntax: _detach(self,[0/1])");
    }
	else {
		pyext *ext = GetThis(self);
		ext->detach = val != 0;
	}

    Py_INCREF(Py_None);
    return Py_None;
}

//! Stop running threads
PyObject *pyext::pyext_stop(PyObject *,PyObject *args)
{
	PyObject *self; 
	int val = -1;
    if(!PyArg_ParseTuple(args, "O|i:pyext_stop",&self,&val)) {
        // handle error
		post("pyext - Syntax: _stop(self,{wait time}");
    }
	else {
		pyext *ext = GetThis(self);
		int cnt = 0;
		t_atom at;
		if(val >= 0) flext::SetInt(at,val);
		ext->m_stop(cnt,&at);
	}

    Py_INCREF(Py_None);
    return Py_None;
}

#endif

//! Query whether threading is enabled
PyObject *pyext::pyext_isthreaded(PyObject *,PyObject *)
{
	return PyInt_FromLong(
#ifdef FLEXT_THREADED
        1
#else
        0
#endif
        );
}

#if FLEXT_SYS == FLEXT_SYS_PD
//! Send message to canvas
PyObject *pyext::pyext_tocanvas(PyObject *,PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

    int sz = PyTuple_GET_SIZE(args);

	bool ok = false;
	PyObject *self; // borrowed ref
	if(
        sz >= 1 &&
        (self = PyTuple_GET_ITEM(args,0)) != NULL && PyInstance_Check(self)
    ) {
		pyext *ext = GetThis(self);

		PyObject *val;

        bool tp = 
            sz == 2 && 
            PySequence_Check(
                val = PyTuple_GET_ITEM(args,1) // borrowed ref
            );

		if(!tp)
			val = PyTuple_GetSlice(args,1,sz);  // new ref

		AtomList *lst = GetPyArgs(val);
		if(lst) {
			t_glist *gl = ext->thisCanvas(); //canvas_getcurrent();
			t_class **cl = (t_pd *)gl;
			if(cl) {
				pd_forwardmess(cl,lst->Count(),lst->Atoms());
			}
#ifdef FLEXT_DEBUG
			else
				post("pyext - no parent canvas?!");
#endif
			ok = true;
		}
		else 
			post("py/pyext - No data to send");
		if(lst) delete lst;

		if(!tp) Py_DECREF(val);
	}

	if(!ok)	post("pyext - Syntax: _tocanvas(self,args...)");

    Py_INCREF(Py_None);
    return Py_None;
}
#endif



