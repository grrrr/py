/* 

py - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"

V lib_setup()
{
	post("py %s - py/pyext python script objects, (C)2002 Thomas Grill",PY__VERSION);
	post("");

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pyext);
}

FLEXT_LIB_SETUP(py,lib_setup)


py::py(): sName(NULL),hName(0) 
{
	// under Max/MSP: doesn't survive next line.....
    if(!(pyref++)) Py_Initialize();
}

py::~py()
{
    if(!(--pyref)) {
		delete modules; modules = NULL;
		Py_Finalize();
	}
}


I py::pyref = 0;
py::lookup *py::modules = NULL;

py::lookup::lookup(I hname,PyObject *mod):
	modhash(hname),
	module(NULL),dict(NULL),
	nxt(NULL)
{
	Set(mod);
}

py::lookup::~lookup()
{
	if(module) Py_DECREF(module);
	if(nxt) delete nxt;
}

V py::lookup::Set(PyObject *mod)
{
    if(module) Py_DECREF(module);
	dict = PyModule_GetDict(module = mod);
}

V py::lookup::Add(lookup *l)
{
	if(nxt) nxt->Add(l);
	else nxt = l;
}



C *py::strdup(const C *s) 
{
	if(!s) return NULL;
	I len = strlen(s);
	C *ret = new C[len+1];
	strcpy(ret,s);
	return ret;
}


V py::SetArgs(I argc,t_atom *argv)
{
	// script arguments
	I i;
	C **sargv = new C *[argc];
	for(i = 0; i < argc; ++i) {
		sargv[i] = new C[256];
		GetAString(argv[i],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc,sargv);

	for(i = 0; i < argc; ++i) delete[] sargv[i];
	delete[] sargv;
}

V py::ImportModule(const C *name)
{
	if(!name) return;

	if(sName) delete[] sName;
	sName = strdup(name);

	PyObject *pName = PyString_FromString(sName);
	hName = PyObject_Hash(pName);

	PyObject *pModule = PyImport_Import(pName);
	if (!pModule) {
//		post("%s: python script %s not found or init error",thisName(),sName);
		PyErr_Print();
	}
	else {
		SetModule(hName,pModule);
	}

	Py_DECREF(pName);
}

V py::SetModule(I hname,PyObject *module)
{
	lookup *l;
	for(l = modules; l && l->modhash != hname; l = l->nxt);

	if(l) 
		l->Set(module);
	else {
		lookup *mod = new lookup(hname,module); 
		if(modules) modules->Add(mod);
		else modules = mod;
	}
}

V py::ReloadModule()
{
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	if(l && l->module) {
		PyObject *newmod = PyImport_ReloadModule(l->module);
		if(!newmod) 
			PyErr_Print();
		else {
			l->Set(newmod);
		}
	}
}

PyObject *py::GetModule()
{
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	return l?l->module:NULL;
}

PyObject *py::GetFunction(const C *func)
{
	if(!func) return NULL;
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	return l?PyDict_GetItemString(l->dict,const_cast<C *>(func)):NULL;
}

PyObject *py::MakePyArgs(const t_symbol *s,I argc,t_atom *argv)
{
	BL any = s && s != sym_bang && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer;

	PyObject *pArgs = PyTuple_New(any?argc+1:argc);

	I ix = 0;

	if(any) {
		PyObject *pValue = PyString_FromString(GetString(s));

		if(!pValue) 
			post("py: cannot convert method header");

		/* pValue reference stolen here: */
		PyTuple_SetItem(pArgs, ix++, pValue); 
	}

	for(I i = 0; i < argc; ++i) {
		PyObject *pValue = NULL;
		
		if(IsFloat(argv[i])) pValue = PyFloat_FromDouble((D)GetFloat(argv[i]));
		else if(IsInt(argv[i])) pValue = PyInt_FromLong(GetInt(argv[i]));
		else if(IsSymbol(argv[i])) pValue = PyString_FromString(GetString(argv[i]));
		else if(IsPointer(argv[i])) pValue = NULL; // not handled

		if(!pValue) {
			post("py: cannot convert argument %i",any?i+1:i);
			continue;
		}

		/* pValue reference stolen here: */
		PyTuple_SetItem(pArgs, ix++, pValue); 
	}

	return pArgs;
}

t_atom *py::GetPyArgs(int &argc,PyObject *pValue)
{
	if(pValue == NULL) { argc = 0; return NULL; }

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

	t_atom *ret = new t_atom[rargc];

	for(I ix = 0; ix < rargc; ++ix) {
		PyObject *arg;
		switch(tp) {
			case tuple: arg = PyTuple_GetItem(pValue,ix); break;
			case list: arg = PyList_GetItem(pValue,ix); break;
			default: arg = pValue;
		}

		if(PyInt_CheckExact(arg)) SetFlint(ret[ix],PyInt_AsLong(arg));
		else if(PyFloat_Check(arg)) SetFloat(ret[ix],(F)PyFloat_AsDouble(arg));
		else if(PyString_Check(arg)) SetString(ret[ix],PyString_AsString(arg));
		else {
			post("py: Could not convert return argument");
			ok = false;
		}
		// No DECREF for arg -> borrowed from pValue!
	}

	if(!ok) { 
		delete[] ret; 
		argc = 0; 
		return NULL; 
	}
	else {
		argc = rargc;
		return ret;
	}
}




