/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


void pybase::Register(const char *regnm)
{
	if(module) {
		// add this to module registry

		PyObject *reg = PyDict_GetItemString(dict,(char *)regnm); // borrowed!!!
		PyObject *add = Py_BuildValue("[i]",(long)this);
		if(!reg || !PyList_Check(reg)) {
			if(PyDict_SetItemString(dict,(char *)regnm,add)) {
				post("py/pyext - Could not set registry");
			}
		}
		else {
			PySequence_InPlaceConcat(reg,add);
		}
	}
}

void pybase::Unregister(const char *regnm)
{
	if(module) {
		// remove this from module registry
		
		PyObject *reg = PyDict_GetItemString(dict,(char *)regnm); // borrowed!!!
		PyObject *add = Py_BuildValue("i",(int)this);
		if(!reg || !PySequence_Check(reg)) 
			post("py/pyext - Internal error: Registry not found!?");
		else {
			int ix = PySequence_Index(reg,add);
			if(ix < 0) {
                post("py/pyext - Internal error: object not found in registry?!");
			}
			else {	
				PySequence_DelItem(reg,ix);
			}
		}
		Py_DECREF(add);
	}
}

void pybase::Reregister(const char *regnm)
{
	if(module) {
		// remove this from module registry
		
		PyObject *reg = PyDict_GetItemString(dict,(char *)regnm); // borrowed!!!

		if(!reg || !PySequence_Check(reg)) 
			post("py/pyext - Internal error: Registry not found!?");
		else {
			int cnt = PySequence_Size(reg);
			for(int i = 0; i < cnt; ++i) {
				PyObject *it = PySequence_GetItem(reg,i); // new reference
				if(!it || !PyInt_Check(it)) {
                    post("py/pyext - Internal error: Corrupt registry?!");
				}
				else {
					pybase *th = (pybase *)PyInt_AsLong(it);
					th->module = module;
					th->dict = dict;
					th->Reload();
				}

                Py_XDECREF(it);
			}
		}
	}
}
