/* 

py - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#include <flext.h>
#include <Python.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 200)
#error You need at least flext version 0.2.0 
#endif


class py:
	public flext_base
{
	FLEXT_HEADER(py,flext_base)

public:
	py(I argc,t_atom *argv);
	~py();

protected:
	virtual V m_method_m(I n,const t_symbol *s,I argc,t_atom *argv); 

    PyObject *pName,*pModule,*pDict,*pFunc;	
	t_atom *ret;
};

// make implementation of a tilde object with one float arg
FLEXT_GIMME("py",py)


py::py(I argc,t_atom *argv):
	pName(NULL),pModule(NULL),pDict(NULL),pFunc(NULL),
	ret(NULL)
{ 
	add_in_anything();  
	add_out_anything();  
	setup_inout();  // set up inlets and outlets


    Py_Initialize();

    if (argc < 2 || !is_symbol(argv[0]) || !is_symbol(argv[1])) {
        post("%s: Syntax: %s pythonfile function",thisName(),thisName());
    }
	else {
		const C *scrname = geta_string(argv[0]);

		pName = PyString_FromString(scrname);

		pModule = PyImport_Import(pName);
		if (!pModule) 
			post("%s: python script %s not found",thisName(),scrname);
		else {
			pDict = PyModule_GetDict(pModule);
			/* pDict is a borrowed reference */

			pFunc = PyDict_GetItemString(pDict,(C *)geta_string(argv[1]));
			/* pFun: Borrowed reference */
		}
	}
}

py::~py()
{
	if(ret) delete[] ret;
    
	// pDict and pFunc are borrowed and must not be Py_DECREF-ed 

    if(pModule) Py_DECREF(pModule);
    if(pName) Py_DECREF(pName);

    Py_Finalize();
}

V py::m_method_m(I,const t_symbol *s,I argc,t_atom *argv)
{
    PyObject *pArgs, *pValue;

	if(pFunc && PyCallable_Check(pFunc)) {

		I pyargc = argc;
		if(s && (s != sym_float || s != sym_int || s != sym_symbol || s != sym_list)) ++pyargc;

		pArgs = PyTuple_New(pyargc);

		I ix = 0;
		if(s && (s != sym_float || s != sym_int || s != sym_symbol || s != sym_list)) {
			pValue = PyString_FromString(s->s_name);
			if(!pValue) {
				post("%s: cannot convert argument header",thisName());
			}

			/* pValue reference stolen here: */
			PyTuple_SetItem(pArgs, ix, pValue);
			ix++;
		}

		for(I i = 0; i < argc; ++i) {
			pValue = NULL;
			
			if(is_float(argv[i])) pValue = PyFloat_FromDouble((D)get_float(argv[i]));
			else if(is_int(argv[i])) pValue = PyInt_FromLong(get_int(argv[i]));
			if(is_symbol(argv[i])) pValue = PyString_FromString(geta_string(argv[i]));

			if(!pValue) {
				post("%s: cannot convert argument",thisName());
				continue;
			}

			/* pValue reference stolen here: */
			PyTuple_SetItem(pArgs, ix, pValue);
			++ix;
		}

		pValue = PyObject_CallObject(pFunc, pArgs);
		if (pValue != NULL) {
			// analyze return tuple

			PyObject *arg;
			int rargc;
			if(PyTuple_Check(pValue)) 
				rargc = PyTuple_Size(pValue);
			else {
				arg = pValue;
				rargc = -1;
			}

			if(ret) delete[] ret;
			ret = new t_atom[rargc < 0?1:rargc];

			for(int ix = 0; ix < rargc || rargc < 0; ++ix) {
				if(rargc >= 0) arg = PyTuple_GetItem(pValue,ix);

				if(PyInt_CheckExact(arg)) set_flint(ret[ix],PyInt_AsLong(arg));
				else if(PyFloat_Check(arg)) set_float(ret[ix],PyFloat_AsDouble(arg));
				else if(PyString_Check(arg)) set_string(ret[ix],PyString_AsString(arg));
				else 
					post("%s: Could not convert return argument",thisName());

				if(rargc < 0) break;

				Py_DECREF(arg);
			}

			Py_DECREF(pValue);

			to_out_list(0,rargc < 0?1:rargc,ret);
		}
		else {
			post("%s: python function call failed",thisName());
		}
		Py_DECREF(pArgs);
	}
	else {
		post("%s: no function defined",thisName());
	}
}
