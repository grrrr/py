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


py::py(): 
	sName(NULL),hName(0),
	detach(false),waittime(100)
{
	Lock();
	// under Max/MSP: doesn't survive next line.....

	if(!(pyref++)) {
		Py_Initialize();
#ifdef FLEXT_THREADS
		PyEval_InitThreads();
#endif
	}

	Unlock();
}

py::~py()
{
	Lock();

    if(!(--pyref)) {
		delete modules; modules = NULL;
#ifdef FLEXT_THREADS
		PyEval_ReleaseLock();
#endif
		Py_Finalize();
	}

	Unlock();
}


I py::pyref = 0;
py::lookup *py::modules = NULL;

py::lookup::lookup(I hname,PyObject *mod,py *_th):
	modhash(hname),
	module(NULL),dict(NULL),
	nxt(NULL)
{
	Set(mod,_th);
}

py::lookup::~lookup()
{
	if(module) Py_DECREF(module);
	if(nxt) delete nxt;
}

V py::lookup::Set(PyObject *mod,py *_th)
{
    if(module) Py_DECREF(module);
	dict = PyModule_GetDict(module = mod);
	th = _th;
}

V py::lookup::Add(lookup *l)
{
	if(nxt) nxt->Add(l);
	else nxt = l;
}

py *py::lookup::GetThis(PyObject *mod)
{
    if(module == mod) return th;
	else
		return nxt?nxt->GetThis(mod):NULL;
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
	// Py_BEGIN_ALLOW_THREADS

	// script arguments
	I i;
	C **sargv = new C *[argc+1];
	for(i = 0; i <= argc; ++i) {
		sargv[i] = new C[256];
		if(!i) 
			strcpy(sargv[i],thisName());
		else
			GetAString(argv[i-1],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc+1,sargv);

	// Py_END_ALLOW_THREADS

	for(i = 0; i <= argc; ++i) delete[] sargv[i];
	delete[] sargv;
}

V py::ImportModule(const C *name)
{
	if(!name) return;

	if(sName) delete[] sName;
	sName = strdup(name);

	// Py_BEGIN_ALLOW_THREADS

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
	// Py_END_ALLOW_THREADS
}

V py::SetModule(I hname,PyObject *module)
{
	// Py_BEGIN_ALLOW_THREADS
	Lock();

	lookup *l;
	for(l = modules; l && l->modhash != hname; l = l->nxt);

	if(l) 
		l->Set(module,this);
	else {
		lookup *mod = new lookup(hname,module,this); 
		if(modules) modules->Add(mod);
		else modules = mod;
	}

	Unlock();
	// Py_END_ALLOW_THREADS
}

V py::ReloadModule()
{
	// Py_BEGIN_ALLOW_THREADS
	Lock();

	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	if(l && l->module) {
		PyObject *newmod = PyImport_ReloadModule(l->module);
		if(!newmod) 
			PyErr_Print();
		else {
			l->Set(newmod,this);
		}
	}

	Unlock();
	// Py_END_ALLOW_THREADS
}

PyObject *py::GetModule()
{
	Lock();

	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	PyObject *ret = l?l->module:NULL;

	Unlock();
	return ret;
}

PyObject *py::GetDict()
{
	Lock();

	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	PyObject *ret = l?l->dict:NULL;

	Unlock();
	return ret;
}

PyObject *py::GetFunction(const C *func)
{
	// Py_BEGIN_ALLOW_THREADS

	PyObject *ret = NULL;
	if(func) {
		Lock();

		lookup *l;
		for(l = modules; l && l->modhash != hName; l = l->nxt);
		ret = l?PyDict_GetItemString(l->dict,const_cast<C *>(func)):NULL;

		Unlock();
	}

	// Py_END_ALLOW_THREADS
	return ret;
}

PyObject *py::MakePyArgs(const t_symbol *s,I argc,t_atom *argv,I inlet)
{
	// Py_BEGIN_ALLOW_THREADS

	BL any = IsAnything(s);

	PyObject *pArgs = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	I pix = 0;

	if(inlet >= 0) {
		PyObject *pValue = PyInt_FromLong(inlet);
		/* pValue reference stolen here: */
		PyTuple_SetItem(pArgs, pix++, pValue); 
	}

	I ix;
	PyObject *tmp;
	if(argc < (any?1:2)) tmp = pArgs,ix = pix;
	else tmp = PyTuple_New(argc+(any?1:0)),ix = 0;

	if(any) {
		PyObject *pValue = PyString_FromString(GetString(s));

		/* pValue reference stolen here: */
		PyTuple_SetItem(tmp, ix++, pValue); 
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
		PyTuple_SetItem(tmp, ix++, pValue); 
	}

	if(tmp != pArgs) {
		PyTuple_SetItem(pArgs, pix++, tmp); 
		_PyTuple_Resize(&pArgs,pix);
	}

	// Py_END_ALLOW_THREADS

	return pArgs;
}

t_atom *py::GetPyArgs(int &argc,PyObject *pValue,PyObject **self)
{
	if(pValue == NULL) { argc = 0; return NULL; }

	// Py_END_ALLOW_THREADS

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
		else if(ix == 0 && self && PyInstance_Check(arg)) {
			// assumed to be self ... that should be checked _somehow_ !!!
			*self = arg;
		}
		else {
			post("py: Could not convert return argument");
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

	// Py_END_ALLOW_THREADS

	return ret;
}


V py::GetModulePath(const C *mod,C *dir,I len)
{
#ifdef PD
	// uarghh... pd doesn't show it's path for extra modules

	C *name;
	I fd = open_via_path("",mod,".py",dir,&name,len,0);
	if(fd > 0) close(fd);
	else name = NULL;

	// if dir is current working directory... name points to dir
	if(dir == name) strcpy(dir,".");
#elif defined(MAXMSP)
	*dir = 0;
#endif
}

V py::AddToPath(const C *dir)
{
	if(dir && *dir) {
		// Py_END_ALLOW_THREADS

		PyObject *pobj = PySys_GetObject("path");
		if(pobj && PyList_Check(pobj)) {
			PyObject *ps = PyString_FromString(dir);
			PyList_Append(pobj,ps);
		}
		PySys_SetObject("path",pobj);

		// Py_END_ALLOW_THREADS
	}
}


//flext_base::ThrMutex py::mutex;

#ifdef FLEXT_THREADS
BL py::Trylock(I wait) 
{
	if(mutex.TryLock()) {
		Sleep((F)((wait >= 0?wait:waittime)/1000.));

		if(mutex.TryLock()) {
			post("%s - A thread is running!",thisName());
			return false;
		}
	}
	return true;
}	
#endif
