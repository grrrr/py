/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002-2004 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"
#include "flinternal.h"

#include <set>

typedef std::set<PyObject *> FuncSet;

struct bounddata 
{ 
    PyObject *self;
    FuncSet funcs;
};

bool pyext::boundmeth(flext_base *,t_symbol *sym,int argc,t_atom *argv,void *data)
{
    bounddata *obj = (bounddata *)data;

	PY_LOCK

	PyObject *args = MakePyArgs(sym,argc,argv,-1,obj->self != NULL);

    // call all functions bound by this symbol
    for(FuncSet::iterator it = obj->funcs.begin(); it != obj->funcs.end(); ++it) {
	    PyObject *ret = PyObject_CallObject(*it,args);
	    if(!ret) {
		    PyErr_Print();
	    }
    	Py_XDECREF(ret);
    }

    Py_XDECREF(args);

	PY_UNLOCK
    return true;
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
		const t_symbol *recv = MakeSymbol(name);
/*
		if(GetBound(recv))
			post("py/pyext - Symbol \"%s\" is already hooked",GetString(recv));
*/		
		// make a proxy object

		if(PyMethod_Check(meth)) {
			PyObject *no = PyObject_GetAttrString(meth,"__name__");
			meth  = PyObject_GetAttr(self,no); 
			Py_DECREF(no);
		}

        void *data = NULL;
        if(GetThis(self)->GetBoundMethod(recv,boundmeth,data)) {
            // already bound to that symbol and function
            bounddata *bdt = (bounddata *)data;
            FLEXT_ASSERT(bdt != NULL && bdt->self == self);
            bdt->funcs.insert(meth);
        }
        else {
            bounddata *data = new bounddata;
            data->self = self;
            data->funcs.insert(meth);
            GetThis(self)->BindMethod(recv,boundmeth,data);

    		Py_INCREF(self); // self is borrowed reference
        }
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
		const t_symbol *recv = MakeSymbol(name);
		if(PyMethod_Check(meth)) {
			PyObject *no = PyObject_GetAttrString(meth,"__name__");
			meth  = PyObject_GetAttr(self,no); // meth is given a new reference!
			Py_DECREF(no);
		}

        void *data = NULL;
        if(GetThis(self)->UnbindMethod(recv,boundmeth,&data)) {
            bounddata *bdt = (bounddata *)data;
            FLEXT_ASSERT(bdt != NULL);

    	    if(PyMethod_Check(meth)) Py_DECREF(meth);

            // erase from map
            bdt->funcs.erase(meth);

            if(bdt->funcs.empty()) {
    		    Py_DECREF(bdt->self);
                delete bdt; 
            }
        }
	}

    Py_INCREF(Py_None);
    return Py_None;
}


V pyext::ClearBinding()
{
    // in case the object couldn't be constructed...
    if(!pyobj) return;

    FLEXT_ASSERT(GetThis(pyobj));

    void *data = NULL;
    const t_symbol *sym = NULL;

    // unbind all 
    while(GetThis(pyobj)->UnbindMethod(sym,NULL,&data)) {
        bounddata *bdt = (bounddata *)data; 
        if(bdt) {
            for(FuncSet::iterator it = bdt->funcs.begin(); it != bdt->funcs.end(); ++it) {
                PyObject *func = *it;
		        if(PyMethod_Check(func)) Py_DECREF(func);
            }

		    Py_DECREF(bdt->self);
            if(data) delete bdt; 
        }
    }
}
