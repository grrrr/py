/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


PyObject *py::MakePyArgs(const t_symbol *s,const AtomList &args,I inlet,BL withself)
{
	PyObject *pArgs;

	BL any = IsAnything(s);
	pArgs = PyTuple_New(args.Count()+(any?1:0)+(inlet >= 0?1:0));

	I pix = 0;

	if(inlet >= 0) {
		PyObject *pValue = PyInt_FromLong(inlet); 
		
		// reference stolen:
		PyTuple_SetItem(pArgs, pix++, pValue); 
	}

	I ix;
	PyObject *tmp;
	if(!withself || args.Count() < (any?1:2)) tmp = pArgs,ix = pix;
	else tmp = PyTuple_New(args.Count()+(any?1:0)),ix = 0;

	if(any) {
		PyObject *pValue = PyString_FromString(GetString(s));

		// reference stolen here: 
		PyTuple_SetItem(tmp, ix++, pValue); 
	}

	for(I i = 0; i < args.Count(); ++i) {
		PyObject *pValue = NULL;
		
		if(IsFloat(args[i])) pValue = PyFloat_FromDouble((D)GetFloat(args[i]));
		else if(IsInt(args[i])) pValue = PyInt_FromLong(GetInt(args[i]));
		else if(IsSymbol(args[i])) pValue = PyString_FromString(GetString(args[i]));
		else if(IsPointer(args[i])) pValue = NULL; // not handled

		if(!pValue) {
			post("py/pyext: cannot convert argument %i",any?i+1:i);
			continue;
		}

		/* pValue reference stolen here: */
		PyTuple_SetItem(tmp, ix++, pValue); 
	}

	if(tmp != pArgs) {
		PyTuple_SetItem(pArgs, pix++, tmp); 
#if PY_VERSION_HEX >= 0x02020000
		_PyTuple_Resize(&pArgs,pix);
#else
		_PyTuple_Resize(&pArgs,pix,0);
#endif
	}

	return pArgs;
}

flext::AtomList *py::GetPyArgs(PyObject *pValue,PyObject **self)
{
	if(pValue == NULL) return NULL; 
	AtomList *ret = NULL;

	// analyze return value or tuple

	I rargc = 0;
	BL ok = true;
	retval tp = nothing;

	if(PyString_Check(pValue)) {
		rargc = 1;
		tp = atom;
	}
	else if(PySequence_Check(pValue)) {
		rargc = PySequence_Size(pValue);
		tp = sequ;
	}
	else {
		rargc = 1;
		tp = atom;
	}

	ret = new AtomList(rargc);

	for(I ix = 0; ix < rargc; ++ix) {
		PyObject *arg;
		switch(tp) {
			case sequ: arg = PySequence_GetItem(pValue,ix); break;
			default: arg = pValue;
		}

		if(PyInt_Check(arg)) SetInt((*ret)[ix],PyInt_AsLong(arg));
		else if(PyLong_Check(arg)) SetInt((*ret)[ix],PyLong_AsLong(arg));
		else if(PyFloat_Check(arg)) SetFloat((*ret)[ix],(F)PyFloat_AsDouble(arg));
		else if(PyString_Check(arg)) SetString((*ret)[ix],PyString_AsString(arg));
		else if(ix == 0 && self && PyInstance_Check(arg)) {
			// assumed to be self ... that should be checked _somehow_ !!!
			*self = arg;
		}
		else {
			PyObject *tp = PyObject_Type(arg);
			PyObject *stp = tp?PyObject_Str(tp):NULL;
			C *tmp = "";
			if(stp) tmp = PyString_AsString(stp);
			post("py/pyext: Could not convert argument %s",tmp);
			Py_XDECREF(stp);
			Py_XDECREF(tp);
			ok = false;
		}
		// No DECREF for arg -> borrowed from pValue!
	}

	if(!ok) { 
		delete ret; 
		ret = NULL; 
	}
	return ret;
}

