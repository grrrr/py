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
#if 1
    else if(flext::CanbeInt(at) || flext::CanbeFloat(at)) {
        // if a number can be an integer... let it be an integer!
        int ival = flext::GetAInt(at);
        double fval = flext::GetAFloat(at);
        return (double)ival == fval?PyInt_FromLong(ival):PyFloat_FromDouble(fval);
    }
#else
    else if(flext::IsFloat(at))
        return PyFloat_FromDouble(flext::GetFloat(at));
    else if(flext::IsInt(at))
        return PyInt_FromLong(flext::GetInt(at));
#endif
    return NULL;
}

static PyObject *MakePyAtom(int argc,const t_atom *argv)
{
    if(argc != sizeof(size_t)/2) return NULL;

    size_t atom = 0;
    for(int i = sizeof(size_t)/2-1; i >= 0; --i)
        if(!flext::CanbeInt(argv[i])) { 
            atom = 0; 
            break; 
        }
        else
            atom = (atom<<16)+flext::GetAInt(argv[i]);

    if(atom) {
        PyObject *el = PyAtom::Retrieve(atom);
        if(!el) el = Py_None; // object already gone....
        Py_INCREF(el);
        return el;
    }
    else
        return NULL;
}

PyObject *pybase::MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,int inlet)
{
	PyObject *ret,*el;

    if(s == symatom && (el = MakePyAtom(argc,argv)) != NULL) {
    	ret = PyTuple_New(1);
		PyTuple_SET_ITEM(ret,0,el);             
    }
    else {
	    bool any = IsAnything(s);
	    ret = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	    int pix = 0;

	    if(inlet >= 0)
		    PyTuple_SET_ITEM(ret,pix++,PyInt_FromLong(inlet)); 

	    if(any)
		    PyTuple_SET_ITEM(ret,pix++,pySymbol_FromSymbol(s)); 

	    for(int i = 0; i < argc; ++i) {
		    el = MakePyAtom(argv[i]);
		    if(!el) {
			    post("py/pyext: cannot convert argument %i",any?i+1:i);
                
                el = Py_None;
                Py_INCREF(Py_None);
            }
		    
		    PyTuple_SET_ITEM(ret,pix++,el); // reference stolen
	    }
    }

	return ret;
}

PyObject *pybase::MakePyArg(const t_symbol *s,int argc,const t_atom *argv)
{
	PyObject *ret;

    if(s == symatom && (ret = MakePyAtom(argc,argv)) != NULL) {
        // ok!
    }
    else if(argc == 1 && !IsAnything(s))
        // convert atoms and one-element lists
		ret = MakePyAtom(*argv);       
    else {
	    bool any = s != sym_list;
	    ret = PyTuple_New(argc+(any?1:0));

	    int pix = 0;
	    if(any)
		    PyTuple_SET_ITEM(ret,pix++,pySymbol_FromSymbol(s)); 

	    for(int i = 0; i < argc; ++i) {
		    PyObject *el = MakePyAtom(argv[i]);
		    if(!el) {
			    post("py/pyext: cannot convert argument %i",any?i+1:i);
                
                el = Py_None;
                Py_INCREF(Py_None);
            }
    		
		    PyTuple_SET_ITEM(ret,pix++,el); // reference stolen
	    }
    }

	return ret;
}

const t_symbol *pybase::GetPyArgs(AtomList &lst,PyObject *pValue,int offs)
{
	if(pValue == NULL) return false; 

    // output bang on None returned
    if(pValue == Py_None) return sym_bang;

	// analyze return value or tuple

	int rargc = 0;
	retval tp = nothing;

	if(PyString_Check(pValue) || pySymbol_Check(pValue)) {
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
//    else
//        Py_DECREF(pValue);

	lst(offs+rargc);

    const t_symbol *sym = NULL;

	for(int ix = 0; ix < rargc; ++ix) {
		PyObject *arg;
		if(tp == sequ)
            arg = PySequence_GetItem(pValue,ix); // new reference
		else
            arg = pValue;

        t_atom &at = lst[offs+ix];
        if(PyInt_Check(arg)) { SetInt(at,PyInt_AsLong(arg)); sym = sym_fint; }
        else if(PyLong_Check(arg)) { SetInt(at,PyLong_AsLong(arg)); sym = sym_fint; }
        else if(PyFloat_Check(arg)) { SetFloat(at,(float)PyFloat_AsDouble(arg)); sym = sym_float; }
        else if(pySymbol_Check(arg)) { SetSymbol(at,pySymbol_AS_SYMBOL(arg)); sym = sym_symbol; }
        else if(PyString_Check(arg)) { SetString(at,PyString_AS_STRING(arg)); sym = sym_symbol; }
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

            SetSymbol(at,sym__); sym = sym_symbol;
		}

		if(tp == sequ) 
            Py_DECREF(arg);
	}

    if(sym && tp == sequ) sym = sym_list;

    return sym;
}


const t_symbol *pybase::GetPyAtom(AtomList &lst,PyObject *obj)
{
    size_t atom = PyAtom::Register(obj);
    size_t szat = sizeof(atom)/2;

    lst(szat);
    for(size_t i = 0; i < szat; ++i,atom >>= 16)
        flext::SetInt(lst[i],(int)(atom&((1<<16)-1)));
    return symatom;
}
