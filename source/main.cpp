/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"

V py::lib_setup()
{
	post("");
	post("py/pyext %s - python script objects, (C)2002 Thomas Grill",PY__VERSION);
	post("");

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pyext);

	pyref = 0;
}

FLEXT_LIB_SETUP(py,py::lib_setup)

PyInterpreterState *py::pystate = NULL;


I py::pyref = 0;
PyObject *py::module_obj = NULL;
PyObject *py::module_dict = NULL;


py::py(): 
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

		pystate = PyThreadState_Get()->interp;
	#endif
		// register/initialize pyext module only once!
		module_obj = Py_InitModule(PYEXT_MODULE, func_tbl);
		module_dict = PyModule_GetDict(module_obj);

		PyModule_AddStringConstant(module_obj,"__doc__",(C *)py_doc);

	#ifdef FLEXT_THREADS
		pythrmain = PyEval_SaveThread();
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

		// Wait for a certain time
		for(int i = 0; i < (PY_STOP_WAIT/PY_STOP_TICK) && thrcount; ++i) Sleep((F)(PY_STOP_TICK/1000.));

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

		Py_XDECREF(module);

//		delete modules; modules = NULL;

		PyEval_AcquireThread(pythrmain); 
		PyThreadState *new_state = PyThreadState_New(pystate); // must have lock 
		PyThreadState *prev_state = PyThreadState_Swap(new_state);

		Py_Finalize();
	}

	Unlock();
*/
	if(clk) clock_free(clk);
}


V py::m_doc()
{
	if(dict) {
		PyObject *docf = PyDict_GetItemString(dict,"__doc__"); // borrowed!!!
		if(docf && PyString_Check(docf)) {
			post("");
			post(PyString_AsString(docf));
		}
	}
}




V py::SetArgs(I argc,const t_atom *argv)
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

	module = PyImport_ImportModule((C *)name);
	if (!module) {
		PyErr_Print();
		dict = NULL;
	}
	else
		dict = PyModule_GetDict(module); // borrowed

}


V py::ReloadModule()
{
	if(module) {
		PyObject *newmod = PyImport_ReloadModule(module);
		if(!newmod) {
			PyErr_Print();
			// old module still exists?!
//			dict = NULL;
		}
		else {
			Py_XDECREF(module);
			module = newmod;
			dict = PyModule_GetDict(module); // borrowed
		}
	}
	else 
		post("%s - No module to reload",thisName());
}

V py::GetModulePath(const C *mod,C *dir,I len)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	// uarghh... pd doesn't show it's path for extra modules

	C *name;
	I fd = open_via_path("",mod,".py",dir,&name,len,0);
	if(fd > 0) close(fd);
	else name = NULL;

	// if dir is current working directory... name points to dir
	if(dir == name) strcpy(dir,".");
#elif FLEXT_SYS == FLEXT_SYS_MAX
	// how do i get the path in Max/MSP?
#else
	*dir = 0;
#endif
}

V py::AddToPath(const C *dir)
{
	if(dir && *dir) {
		PyObject *pobj = PySys_GetObject("path");
		if(pobj && PyList_Check(pobj)) {
			int i,n = PyList_Size(pobj);
			for(i = 0; i < n; ++i) {
				PyObject *pt = PyList_GetItem(pobj,i);
				if(PyString_Check(pt) && !strcmp(dir,PyString_AsString(pt))) break;
			}
			if(i == n) { // string is not yet existent in path
				PyObject *ps = PyString_FromString(dir);
				PyList_Append(pobj,ps);
			}
		}
		PySys_SetObject("path",pobj);
	}
}


