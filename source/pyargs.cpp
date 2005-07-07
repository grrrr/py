/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pybase.h"
#include "pyatom.h"

static const t_symbol *symatom = flext::MakeSymbol(" py ");

static PyObject *MakePyAtom(const t_atom &at)
{
	if(flext::IsSymbol(at)) 
        return pySymbol_FromSymbol(flext::GetSymbol(at));
    else if(flext::CanbeInt(at) || flext::CanbeFloat(at)) {
        // if a number can be an integer... let it be an integer!
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
	PyObject *ret;

    if(s == symatom && argc == sizeof(PyObject *)/2) {
        size_t atom = 0;
        for(int i = sizeof(PyObject *)/2-1; i >= 0; --i)
            if(!flext::CanbeInt(argv[i])) { 
                atom = 0; 
                break; 
            }
            else
                atom = (atom<<16)+flext::GetAInt(argv[i]);

        if(atom) {
            PyObject *el = (PyObject *)atom;
            Py_INCREF(el);

            // atom not needed any more
            PyAtom::Collect();

    	    ret = PyTuple_New(1);
		    PyTuple_SET_ITEM(ret,0,el);             
        }
        else
            ret = NULL;
    }
    else {
	    bool any = IsAnything(s);
	    ret = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	    int pix = 0;

	    if(inlet >= 0)
		    PyTuple_SetItem(ret, pix++, PyInt_FromLong(inlet)); 

	    int ix;
	    PyObject *tmp;
    //	    if(!withself || argc < (any?1:2)) 
            tmp = ret,ix = pix;
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

	    if(tmp != ret) {
		    PyTuple_SET_ITEM(ret, pix++, tmp); 
    #if PY_VERSION_HEX >= 0x02020000
		    _PyTuple_Resize(&ret,pix);
    #else
		    _PyTuple_Resize(&ret,pix,0);
    #endif
	    }
    }

	return ret;
}

bool pybase::GetPyArgs(AtomList &lst,PyObject *pValue,int offs /*,PyObject **self*/)
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
/*
        else if(ix == 0 && self && PyInstance_Check(arg)) {
			// assumed to be self ... that should be checked _somehow_ !!!
            Py_INCREF(arg);
			*self = arg;
		}
*/
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


bool pybase::GetPyAtom(AtomList &lst,PyObject *obj)
{
    PyAtom::Register(obj);

    size_t szat = sizeof(obj)/2,atom = (size_t)obj;
    lst(1+szat);
    SetSymbol(lst[0],symatom);
    for(size_t i = 0; i < szat; ++i,atom >>= 16)
        flext::SetInt(lst[i+1],(int)(atom&((1<<16)-1)));
    return true;
}
