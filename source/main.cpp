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
	FLEXT_HEADER_S(py,flext_base)

public:
	py(I argc,t_atom *argv);
	~py();

protected:
	V m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	V work(const t_symbol *s,I argc,t_atom *argv); 
	
	virtual V m_bang() { work(sym_bang,0,NULL); }
	virtual V m_list(I argc,t_atom *argv) { work(sym_list,argc,argv); }
	virtual V m_float(I argc,t_atom *argv) { work(sym_float,argc,argv); }
	virtual V m_int(I argc,t_atom *argv) { work(sym_int,argc,argv); }
	virtual V m_symbol(I argc,t_atom *argv) { work(sym_symbol,argc,argv); }
	virtual V m_any(const t_symbol *s,I argc,t_atom *argv);

    PyObject *pName,*pModule,*pDict,*pFunc;	

	static I pyref;

private:
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK_G(m_float)
	FLEXT_CALLBACK_G(m_list)
	FLEXT_CALLBACK_G(m_int)
	FLEXT_CALLBACK_G(m_symbol)
	FLEXT_CALLBACK_A(m_any)
};

I py::pyref;

V py::cb_setup(t_class *) 
{
	py::pyref = 0;
}

// make implementation of a tilde object with one float arg
FLEXT_GIMME("py",py)


py::py(I argc,t_atom *argv):
	pName(NULL),pModule(NULL),pDict(NULL),pFunc(NULL)
{ 
	add_in_anything();  
	add_out_anything();  
	setup_inout();  // set up inlets and outlets

	FLEXT_ADDBANG(0,m_bang);
	FLEXT_ADDMETHOD_(0,"float",m_float);
	FLEXT_ADDMETHOD_(0,"int",m_int);
	FLEXT_ADDMETHOD_(0,"symbol",m_symbol);
	FLEXT_ADDMETHOD(0,m_list);
	FLEXT_ADDMETHOD(0,m_any);


    if(!(pyref++)) Py_Initialize();

    if (argc < 2 || !is_symbol(argv[0]) || !is_symbol(argv[1])) {
        post("%s: Syntax: %s pythonfile function",thisName(),thisName());
    }
	else {
		const C *scrname = get_string(argv[0]);

		// script arguments
		I margc = argc > 2?argc-2:0;
		C **margv = new C *[margc];
		for(I i = 0; i < margc; ++i) {
			margv[i] = new C[256];
			geta_string(argv[i+2],margv[i],255);
		}

		PySys_SetArgv(margc,margv);

		for(i = 0; i < margc; ++i) delete[] margv[i];
		delete[] margv;

		C dir[1024];
#ifdef PD
		// uarghh... pd doesn't show it's path for extra modules

		C *name;
		I fd = open_via_path("",scrname,".py",dir,&name,sizeof(dir),0);
		if(fd > 0) close(fd);
		else name = NULL;
#elif defined(MAXMSP)
		*dir = 0;
#endif

		// set script path
		PySys_SetPath(dir);

		// init script module

		pName = PyString_FromString(scrname);

		pModule = PyImport_Import(pName);
		if (!pModule) 
			post("%s: python script %s not found or init error",thisName(),scrname);
		else {
			pDict = PyModule_GetDict(pModule);
			/* pDict is a borrowed reference */

			pFunc = PyDict_GetItemString(pDict,(C *)get_string(argv[1]));
			/* pFun: Borrowed reference */
		}

	}
}

py::~py()
{
	// pDict and pFunc are borrowed and must not be Py_DECREF-ed 

    if(pModule) Py_DECREF(pModule);
    if(pName) Py_DECREF(pName);

    if(!(--pyref)) Py_Finalize();
}

V py::m_method_(I n,const t_symbol *s,I argc,t_atom *argv)
{
	post("%s - no method for type %s",thisName(),get_string(s));
}

V py::m_any(const t_symbol *s,I argc,t_atom *argv)
{
	if(argc == 0) {
		t_atom a;
		set_symbol(a,s);
		work(s,1,&a);
	}
	else
		m_method_(0,s,argc,argv);
}


V py::work(const t_symbol *s,I argc,t_atom *argv)
{
    PyObject *pArgs, *pValue;

//	post("work called: inlet=%i, symbol=%s, argc=%i",inlet,s->s_name,argc);

	if(pFunc && PyCallable_Check(pFunc)) {
		pArgs = PyTuple_New(argc);

		I ix = 0;

		for(I i = 0; i < argc; ++i) {
			pValue = NULL;
			
			if(is_float(argv[i])) pValue = PyFloat_FromDouble((D)get_float(argv[i]));
			else if(is_int(argv[i])) pValue = PyInt_FromLong(get_int(argv[i]));
			else if(is_symbol(argv[i])) pValue = PyString_FromString(get_string(argv[i]));

			if(!pValue) {
				post("%s: cannot convert argument",thisName());
				continue;
			}

			/* pValue reference stolen here: */
			PyTuple_SetItem(pArgs, ix++, pValue); 
		}


		pValue = PyObject_CallObject(pFunc, pArgs);
		if (pValue != NULL) {
			// analyze return value or tuple

			int rargc = 0;
			BL tpl = false;

			if(PyObject_Not(pValue)) rargc = 0;
			else {
				tpl = PyTuple_Check(pValue);
				rargc = tpl?PyTuple_Size(pValue):1;
			}

			t_atom *ret = new t_atom[rargc];

			for(int ix = 0; ix < rargc; ++ix) {
				PyObject *arg = tpl?PyTuple_GetItem(pValue,ix):pValue;

				if(PyInt_CheckExact(arg)) set_flint(ret[ix],PyInt_AsLong(arg));
				else if(PyFloat_Check(arg)) set_float(ret[ix],PyFloat_AsDouble(arg));
				else if(PyString_Check(arg)) set_string(ret[ix],PyString_AsString(arg));
				else {
					post("%s: Could not convert return argument",thisName());
					set_string(ret[ix],"???");
				}
				// No DECREF for arg -> borrowed from pValue!
			}

			Py_DECREF(pValue);

			if(rargc) to_out_list(0,rargc,ret);
			delete[] ret;
		}
		else {
			post("%s: python function call failed",thisName());
		}
		if(pArgs) Py_DECREF(pArgs);
	}
	else {
		post("%s: no function defined",thisName());
	}
}
