#include "main.h"

PyObject *py::MakePyArgs(const t_symbol *s,I argc,t_atom *argv,I inlet,BL withself)
{
	PyObject *pArgs;

	BL any = IsAnything(s);
	pArgs = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	I pix = 0;

	if(inlet >= 0) {
		PyObject *pValue = PyInt_FromLong(inlet); 
		
		// reference stolen:
		PyTuple_SetItem(pArgs, pix++, pValue); 
	}

	I ix;
	PyObject *tmp;
	if(!withself || argc < (any?1:2)) tmp = pArgs,ix = pix;
	else tmp = PyTuple_New(argc+(any?1:0)),ix = 0;

	if(any) {
		PyObject *pValue = PyString_FromString(GetString(s));

		// reference stolen here: 
		PyTuple_SetItem(tmp, ix++, pValue); 
	}

	for(I i = 0; i < argc; ++i) {
		PyObject *pValue = NULL;
		
		if(IsFloat(argv[i])) pValue = PyFloat_FromDouble((D)GetFloat(argv[i]));
		else if(IsInt(argv[i])) pValue = PyInt_FromLong(GetInt(argv[i]));
		else if(IsSymbol(argv[i])) pValue = PyString_FromString(GetString(argv[i]));
		else if(IsPointer(argv[i])) pValue = NULL; // not handled

		if(!pValue) {
			post("py/pyext: cannot convert argument %i",any?i+1:i);
			continue;
		}

		/* pValue reference stolen here: */
		PyTuple_SetItem(tmp, ix++, pValue); 
	}

	if(tmp != pArgs) {
		PyTuple_SetItem(pArgs, pix++, tmp); 
		_PyTuple_Resize(&pArgs,pix);
	}

	return pArgs;
}

t_atom *py::GetPyArgs(int &argc,PyObject *pValue,PyObject **self)
{
	if(pValue == NULL) { argc = 0; return NULL; }
	t_atom *ret = NULL;

	// analyze return value or tuple

	I rargc = 0;
	BL ok = true;
	retval tp = nothing;

	if(!PyObject_Not(pValue)) {
		if(PyTuple_Check(pValue)) {
			rargc = PyTuple_Size(pValue);
			tp = tuple;
		}
		else if(PyList_Check(pValue)) {
			rargc = PyList_Size(pValue);
			tp = list;
		}
		else {
			rargc = 1;
			tp = atom;
		}
	}

	ret = new t_atom[rargc];

	for(I ix = 0; ix < rargc; ++ix) {
		PyObject *arg;
		switch(tp) {
			case tuple: arg = PyTuple_GetItem(pValue,ix); break;
			case list: arg = PyList_GetItem(pValue,ix); break;
			default: arg = pValue;
		}

		if(PyInt_Check(arg)) SetFlint(ret[ix],PyInt_AsLong(arg));
		else if(PyLong_Check(arg)) SetFlint(ret[ix],PyLong_AsLong(arg));
		else if(PyFloat_Check(arg)) SetFloat(ret[ix],(F)PyFloat_AsDouble(arg));
		else if(PyString_Check(arg)) SetString(ret[ix],PyString_AsString(arg));
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
		delete[] ret; 
		argc = 0; 
		ret = NULL; 
	}
	else 
		argc = rargc;

	return ret;
}

