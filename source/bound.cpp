/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"
#include "flinternal.h"

t_class *pyext::px_class;
pyext::py_proxy *pyext::px_head,*pyext::px_tail;

void pyext::py_proxy::px_method(py_proxy *obj,const t_symbol *s,int argc,const t_atom *argv)
{
	PY_LOCK

	PyObject *args = MakePyArgs(s,AtomList(argc,argv),-1,obj->self != NULL);
	PyObject *ret = PyObject_CallObject(obj->func,args);
	if(!ret) {
		PyErr_Print();
	}
	Py_XDECREF(ret);

	PY_UNLOCK
}


PyObject *pyext::pyext_bind(PyObject *,PyObject *args)
{
    PyObject *self,*meth;
	C *name;
    if(!PyArg_ParseTuple(args, "OsO:pyext_bind", &self,&name,&meth))
		post("py/pyext - Wrong arguments!");
	else if(!PyInstance_Check(self) || !(PyMethod_Check(meth) || PyFunction_Check(meth))) {
		post("py/pyext - Wrong argument types!");
    }
	else {
		t_symbol *recv = gensym(name);
/*
		if(GetBound(recv))
			post("py/pyext - Symbol \"%s\" is already hooked",GetString(recv));
*/		
		// make a proxy object
		py_proxy *px = (py_proxy *)object_new(px_class);
		if(PyMethod_Check(meth)) {
			PyObject *no = PyObject_GetAttrString(meth,"__name__");
			meth  = PyObject_GetAttr(self,no); 
			Py_DECREF(no);
		}
		px->init(recv,self,meth);  

		// add it to the list
		if(px_tail) px_tail->nxt = px;
		else px_head = px;
		px_tail = px;

		// Do bind
		pd_bind(&px->obj.ob_pd,recv);  

		Py_INCREF(self); // self is borrowed reference
	}

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *pyext::pyext_unbind(PyObject *,PyObject *args)
{
    PyObject *self,*meth;
	C *name;
    if(!PyArg_ParseTuple(args, "OsO:pyext_bind", &self,&name,&meth))
		post("py/pyext - Wrong arguments!");
	else if(!PyInstance_Check(self) || !(PyMethod_Check(meth) || PyFunction_Check(meth))) {
		post("py/pyext - Wrong argument types!");
    }
	else {
		t_symbol *recv = gensym(name);
		if(PyMethod_Check(meth)) {
			PyObject *no = PyObject_GetAttrString(meth,"__name__");
			meth  = PyObject_GetAttr(self,no); // meth is given a new reference!
			Py_DECREF(no);
		}

		// search proxy object
		py_proxy *pp = NULL,*px = px_head;
		while(px) {
			py_proxy *pn = px->nxt;
			if(recv == px->name && self == px->self && meth == px->func) {
				if(pp)
					pp->nxt = pn;
				else
					px_head = pn;
				if(!pn) px_tail = pp;
				break;
			}
			else pp = px;
			px = pn;
		}

		// do unbind
		if(px) {
			pd_unbind(&px->obj.ob_pd,recv);  
			object_free(px->obj);

			Py_DECREF(self);
			if(PyMethod_Check(meth)) Py_DECREF(meth);
		}
	}

    Py_INCREF(Py_None);
    return Py_None;
}


V pyext::ClearBinding()
{
	// search proxy object
	py_proxy *pp = NULL,*px = px_head;
	while(px) {
		py_proxy *pn = px->nxt;
		if(px->self == pyobj) {
			if(pp)
				pp->nxt = pn;
			else
				px_head = pn;
			if(!pn) px_tail = pp;

			Py_DECREF(px->self);
			if(PyMethod_Check(px->func)) Py_DECREF(px->func);

			pd_unbind(&px->obj.ob_pd,px->name);  
			object_free(px->obj);
		}
		else pp = px;	
		px = pn;
	}
}


