#include "main.h"

t_class *pyext::px_class;
pyext::py_proxy *pyext::px_head,*pyext::px_tail;

void pyext::py_proxy::px_method(py_proxy *obj,const t_symbol *s,int argc,t_atom *argv)
{
	PY_LOCK

	PyObject *args = MakePyArgs(s,argc,argv); //,-1,obj->self != NULL);
	PyObject *ret = PyObject_CallObject(obj->func,args);
	Py_XDECREF(ret);

	PY_UNLOCK
}


PyObject *pyext::py_bind(PyObject *,PyObject *args)
{
    PyObject *o1,*o2,*o3 = NULL;
    if(!PyArg_ParseTuple(args, "OO|O:py_bind", &o1,&o2,&o3)) {
        // handle error
		error("py/pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }

	BL ok = false;
	PyObject *self = NULL;
	if(o3 && PyInstance_Check(o1) && PyString_Check(o2) && PyMethod_Check(o3)) {
		// class method
		self = o1,o1 = o2,o2 = o3;
		ok = true;
	}
	else if(!o3 && PyString_Check(o1) && PyFunction_Check(o2)) {
		// function
		ok = true;
	}

	if(!ok) {
		post("py/pyext - Wrong arguments!");
	}
	else {
		t_symbol *recv = gensym(PyString_AsString(o1));
		if(GetBound(recv))
			post("py/pyext - Symbol \"%s\" is already hooked",GetString(recv));
		else {
			// make a proxy object
		    py_proxy *px = (py_proxy *)object_new(px_class);
			px->init(self,o2);  // proxy for 2nd inlet messages 

			// add it to the list
			if(px_tail) px_tail->nxt = px;
			else px_head = px;
			px_tail = px;

			// Do bind
			pd_bind(&px->obj.ob_pd,recv);  
		}
	}

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *pyext::py_unbind(PyObject *,PyObject *args)
{
    PyObject *o1,*o2,*o3 = NULL;
    if(!PyArg_ParseTuple(args, "OO|O:py_bind", &o1,&o2,&o3)) {
        // handle error
		error("py/pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }

	BL ok = false;
	PyObject *self = NULL;
	if(o3 && PyInstance_Check(o1) && PyString_Check(o2) && PyMethod_Check(o3)) {
		// class method
		self = o1,o1 = o2,o2 = o3;
		ok = true;
	}
	else if(!o3 && PyString_Check(o1) && PyFunction_Check(o2)) {
		// function
		ok = true;
	}

	if(!ok) {
		post("py/pyext - Wrong arguments!");
	}
	else {
		t_symbol *recv = gensym(PyString_AsString(o1));

		// search proxy object
		py_proxy *pp = NULL,*px = px_head;
		while(px) {
			if(px->cmp(self,o2)) {
				py_proxy *pn = px->nxt;
				object_free(px->obj);
				px = pn;
			}
			else 
				pp = px,px = px->nxt;
		}

		// do unbind
		if(px) {
			pd_unbind(&px->obj.ob_pd,recv);  
		}
	}

    Py_INCREF(Py_None);
    return Py_None;
}
