/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"

static PyObject *MakePyAtom(const t_atom &at)
{
	if(flext::IsSymbol(at)) 
        return pySymbol_FromSymbol(flext::GetSymbol(at));
    else if(flext::CanbeInt(at) || flext::CanbeFloat(at)) {
        // if a number can be an integer... let at be an integer!
        int ival = flext::GetAInt(at);
        double fval = flext::GetAFloat(at);
        return (double)ival == fval?PyInt_FromLong(ival):PyFloat_FromDouble(fval);
    }
//	else if(flext::IsPointer(at)) return NULL; // not handled
/*
    // these following should never happen
    else if(flext::IsFloat(at)) return PyFloat_FromDouble((double)flext::GetFloat(at));
	else if(flext::IsInt(at)) return PyInt_FromLong(flext::GetInt(at));
*/  
    return NULL;
}

PyObject *pybase::MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,int inlet,bool withself)
{
	PyObject *pArgs;

/*
    if(!s && args.Count() == 1) {
        pArgs = MakePyAtom(args[0]);
        if(!pArgs)
			post("py/pyext: cannot convert argument(s)");
    }
    else 
*/
    {
	    bool any = IsAnything(s);
	    pArgs = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	    int pix = 0;

	    if(inlet >= 0)
		    PyTuple_SetItem(pArgs, pix++, PyInt_FromLong(inlet)); 

	    int ix;
	    PyObject *tmp;
//	    if(!withself || argc < (any?1:2)) 
            tmp = pArgs,ix = pix;
//	    else 
//            tmp = PyTuple_New(argc+(any?1:0)),ix = 0;

	    if(any)
		    PyTuple_SET_ITEM(tmp, ix++, pySymbol_FromSymbol(s)); 

	    for(int i = 0; i < argc; ++i) {
		    PyObject *pValue = MakePyAtom(argv[i]);
		    if(!pValue) {
			    post("py/pyext: cannot convert argument %i",any?i+1:i);
			    continue;
		    }

		    /* pValue reference stolen here: */
		    PyTuple_SET_ITEM(tmp, ix++, pValue); 
	    }

	    if(tmp != pArgs) {
		    PyTuple_SET_ITEM(pArgs, pix++, tmp); 
    #if PY_VERSION_HEX >= 0x02020000
		    _PyTuple_Resize(&pArgs,pix);
    #else
		    _PyTuple_Resize(&pArgs,pix,0);
    #endif
	    }
    }

	return pArgs;
}

bool pybase::GetPyArgs(AtomList &lst,PyObject *pValue,int offs,PyObject **self)
{
	if(pValue == NULL) return false; 

	// analyze return value or tuple

	int rargc = 0;
	bool ok = true;
	retval tp = nothing;

	if(PyString_Check(pValue)) {
		rargc = 1;
		tp = atom;
	}
	else if(PySequence_Check(pValue)) {
		rargc = PySequence_Size(pValue);
		tp = sequ;
	}
    else if(pValue != Py_None) {
		rargc = 1;
		tp = atom;
	}
//    else
//        Py_DECREF(pValue);

	lst(offs+rargc);

	for(int ix = 0; ix < rargc; ++ix) {
		PyObject *arg;
		if(tp == sequ)
            arg = PySequence_GetItem(pValue,ix); // new reference
		else
            arg = pValue;

        t_atom &at = lst[offs+ix];
		if(PyInt_Check(arg)) SetInt(at,PyInt_AsLong(arg));
		else if(PyLong_Check(arg)) SetInt(at,PyLong_AsLong(arg));
		else if(PyFloat_Check(arg)) SetFloat(at,(float)PyFloat_AsDouble(arg));
		else if(pySymbol_Check(arg)) SetSymbol(at,pySymbol_AS_SYMBOL(arg));
		else if(PyString_Check(arg)) SetString(at,PyString_AS_STRING(arg));
		else if(ix == 0 && self && PyInstance_Check(arg)) {
			// assumed to be self ... that should be checked _somehow_ !!!
            Py_INCREF(arg);
			*self = arg;
		}
		else {
			PyObject *tp = PyObject_Type(arg);
			PyObject *stp = tp?PyObject_Str(tp):NULL;
			char *tmp = "";
			if(stp) tmp = PyString_AS_STRING(stp);
			post("py/pyext: Could not convert argument %s",tmp);
			Py_XDECREF(stp);
			Py_XDECREF(tp);
			ok = false;
		}

		if(tp == sequ) 
            Py_DECREF(arg);
	}

    return ok;
}
