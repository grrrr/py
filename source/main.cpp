/* 

py - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"

#define STOP_WAIT 1000  // ms
#define STOP_TICK 10  // ms

V lib_setup()
{
	post("py/pyext %s - python script objects, (C)2002 Thomas Grill",PY__VERSION);
	post("");

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pyext);
}

FLEXT_LIB_SETUP(py,lib_setup)

PyInterpreterState *py::pystate = NULL;

PyMethodDef py::func_tbl[] = 
{
	{ "_samplerate", py::py_samplerate, NULL,"get sample rate" },
	{ "_blocksize", py::py_blocksize, NULL,"get block size" },
	{ "_inchannels", py::py_inchannels, NULL,"get number of audio in channels" },
	{ "_outchannels", py::py_outchannels, NULL,"get number of audio out channels" },
	{NULL, NULL, 0, NULL}
};

PyObject *py::module_obj = NULL;
PyObject *py::module_dict = NULL;


py::py(): 
	sName(NULL),hName(0),
	module(NULL),
	detach(false),shouldexit(false),thrcount(0),
	clk(NULL),stoptick(0)
{
	Lock();
	// under Max/MSP: doesn't survive next line.....

	if(!(pyref++)) {
		Py_Initialize();
#ifdef FLEXT_THREADS
		PyEval_InitThreads();
		pythrmain = PyThreadState_Get();
		pystate = pythrmain->interp;
#endif
		// register/initialize pyext module only once!
		module_obj = Py_InitModule(PYEXT_MODULE, func_tbl);
		module_dict = PyModule_GetDict(module_obj);

#ifdef FLEXT_THREADS
		PyEval_ReleaseLock();
#endif
	}
	else {
		PY_LOCK
		Py_INCREF(module_obj);
		Py_INCREF(module_dict);
		PY_UNLOCK
	}

	Unlock();

	clk = clock_new(this,(t_method)tick);
}

py::~py()
{
	if(thrcount) {
		shouldexit = true;

		// Wait for 0.5 seconds
		for(int i = 0; i < (STOP_WAIT/STOP_TICK) && thrcount; ++i) Sleep((F)(STOP_TICK/1000.));

		// Wait forever
		post("%s - Waiting for thread termination!",thisName());
		while(thrcount) Sleep(0.2f);
		post("%s - Okay, all threads have terminated",thisName());
	}
		
/*
	// don't unregister

	Lock();

    if(!(--pyref)) {
		Py_DECREF(module_obj);
		module_obj = NULL;
		Py_DECREF(module_dict);
		module_dict = NULL;

		delete modules; modules = NULL;

		PyEval_AcquireLock();
	    PyThreadState *new_state = PyThreadState_New(pystate); // must have lock 
		PyThreadState *prev_state = PyThreadState_Swap(new_state);

		Py_Finalize();
	}

	Unlock();
*/
	if(clk) clock_free(clk);
}


I py::pyref = 0;

#if 0
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
	Py_XDECREF(module);
	if(nxt) delete nxt;
}

V py::lookup::Set(PyObject *mod,py *_th)
{
    Py_XDECREF(module);
    Py_XDECREF(dict);
	dict = PyModule_GetDict(module = mod);
	th = _th;
}

V py::lookup::Add(lookup *l)
{
	if(nxt) nxt->Add(l);
	else nxt = l;
}

/*
py *py::lookup::GetThis(PyObject *mod)
{
    if(module == mod) return th;
	else
		return nxt?nxt->GetThis(mod):NULL;
}
*/

#endif

V py::SetArgs(I argc,t_atom *argv)
{
	// script arguments
	C **sargv = new C *[argc+1];
	for(int i = 0; i <= argc; ++i) {
		sargv[i] = new C[256];
		if(!i) 
			strcpy(sargv[i],thisName());
		else
			GetAString(argv[i-1],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc+1,sargv);

	for(int j = 0; j <= argc; ++j) delete[] sargv[j];
	delete[] sargv;
}

V py::ImportModule(const C *name)
{
	if(!name) return;

	if(sName) delete[] sName;
	sName = (C *)flext::strdup(name);

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
}

V py::ReloadModule()
{
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
	PyObject *ret = NULL;

	if(func) {
		Lock();

		lookup *l;
		for(l = modules; l && l->modhash != hName; l = l->nxt);
		ret = l?PyDict_GetItemString(l->dict,const_cast<C *>(func)):NULL;

		Unlock();
	}

	return ret;
}

PyObject *py::MakePyArgs(const t_symbol *s,I argc,t_atom *argv,I inlet,BL withself)
{
	PyObject *pArgs;

	BL any = IsAnything(s);
	pArgs = PyTuple_New(argc+(any?1:0)+(inlet >= 0?1:0));

	I pix = 0;

	if(inlet >= 0) {
		PyObject *pValue = PyInt_FromLong(inlet);
		/* pValue reference stolen here: */
		PyTuple_SetItem(pArgs, pix++, pValue); 
	}

	I ix;
	PyObject *tmp;
	if(!withself || argc < (any?1:2)) tmp = pArgs,ix = pix;
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
		PyObject *pobj = PySys_GetObject("path");
		if(pobj && PyList_Check(pobj)) {
			PyObject *ps = PyString_FromString(dir);
			PyList_Append(pobj,ps);
		}
		PySys_SetObject("path",pobj);
	}
}

V py::tick(py *th)
{
	th->Lock();

	if(!th->thrcount) {
		// all threads have stopped
		th->shouldexit = false;
		th->stoptick = 0;
	}
	else {
		// still active threads 
		if(!--th->stoptick) {
				post("%s - Threads couldn't be stopped entirely - %i remaining",th->thisName(),th->thrcount);
			th->shouldexit = false;
		}
		else
			// continue waiting
			clock_delay(th->clk,STOP_TICK);
	}

	th->Unlock();
}

V py::m_stop(int argc,t_atom *argv)
{
	if(thrcount) {
		Lock();

		I wait = STOP_WAIT;
		if(argc >= 1 && CanbeInt(argv[0])) wait = GetAInt(argv[0]);

		I ticks = wait/STOP_TICK;
		if(stoptick) {
			// already stopping
			if(ticks < stoptick) stoptick = ticks;
		}
		else
			stoptick = ticks;
		shouldexit = true;
		clock_delay(clk,STOP_TICK);

		Unlock();
	}
		
}

PyObject *py::py_samplerate(PyObject *self,PyObject *args)
{
	return PyFloat_FromDouble(sys_getsr());
}

PyObject *py::py_blocksize(PyObject *self,PyObject *args)
{
	return PyLong_FromLong(sys_getblksize());
}

PyObject *py::py_inchannels(PyObject *self,PyObject *args)
{
#ifdef PD
	I ch = sys_get_inchannels();
#else // MAXMSP
	I ch = sys_getch(); // not functioning
#endif
	return PyLong_FromLong(ch);
}

PyObject *py::py_outchannels(PyObject *self,PyObject *args)
{
#ifdef PD
	I ch = sys_get_outchannels();
#else // MAXMSP
	I ch = sys_getch(); // not functioning
#endif
	return PyLong_FromLong(ch);
}



