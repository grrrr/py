/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002-2004 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


V py::Register(const C *regnm)
{
	if(module) {
		// add this to module registry

		PyObject *reg = PyDict_GetItemString(dict,(C *)regnm); // borrowed!!!
		PyObject *add = Py_BuildValue("[i]",(long)this);
		if(!reg || !PyList_Check(reg)) {
			if(PyDict_SetItemString(dict,(C *)regnm,add)) {
				post("%s - Could not set registry",thisName());
			}
			// Py_XDECREF(reg);
		}
		else {
			PySequence_InPlaceConcat(reg,add);
		}
	}

}

V py::Unregister(const C *regnm)
{
	if(module) {
		// remove this from module registry
		
		PyObject *reg = PyDict_GetItemString(dict,(C *)regnm); // borrowed!!!
		PyObject *add = Py_BuildValue("i",(int)this);
		if(!reg || !PySequence_Check(reg)) 
			post("%s - Registry not found!?",thisName());
		else {
			I ix = PySequence_Index(reg,add);
			if(ix < 0) {
				post("%s - This object not found in registry?!",thisName());
			}
			else {	
				PySequence_DelItem(reg,ix);
			}
		}
		Py_DECREF(add);
	}

}

V py::Reregister(const C *regnm)
{
	if(module) {
		// remove this from module registry
		
		PyObject *reg = PyDict_GetItemString(dict,(C *)regnm); // borrowed!!!

		if(!reg || !PySequence_Check(reg)) 
			post("%s - Registry not found!?",thisName());
		else {
			I cnt = PySequence_Size(reg);
			for(I i = 0; i < cnt; ++i) {
				PyObject *it = PySequence_GetItem(reg,i); // new reference
				if(!it || !PyInt_Check(it)) {
					post("%s - Corrupt registry?!",thisName());
				}
				else {
					py *th = (py *)PyInt_AsLong(it);
					th->module = module;
					th->dict = dict;
					th->Reload();
				}

                Py_XDECREF(it);
			}
		}
	}

}



