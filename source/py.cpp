/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


class pyobj:
	public py
{
	FLEXT_HEADER_S(pyobj,py,Setup)

public:
	pyobj(I argc,const t_atom *argv);
	~pyobj();

protected:
	BL m_method_(I n,const t_symbol *s,I argc,const t_atom *argv);

	BL work(const t_symbol *s,I argc,const t_atom *argv); 

	V m_bang() { callwork(sym_bang,0,NULL); }
	V m_reload();
	V m_reload_(I argc,const t_atom *argv);
	V m_set(I argc,const t_atom *argv);
    V m_dir_() { m__dir(function); }
    V m_doc_() { m__doc(function); }

	virtual V m_help();

	// methods for python arguments
	V callwork(const t_symbol *s,I argc,const t_atom *argv);
	
	V m_py_list(I argc,const t_atom *argv) { callwork(sym_list,argc,argv); }
	V m_py_float(I argc,const t_atom *argv) { callwork(sym_float,argc,argv); }
	V m_py_int(I argc,const t_atom *argv) { callwork(sym_int,argc,argv); }
	V m_py_any(const t_symbol *s,I argc,const t_atom *argv) { callwork(s,argc,argv); }

	const t_symbol *funname;
	PyObject *function;

	virtual V Reload();

	V SetFunction(const C *func);
	V ResetFunction();

private:
	static void Setup(t_classid c);

	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK(m_reload)
	FLEXT_CALLBACK_V(m_reload_)
	FLEXT_CALLBACK_V(m_set)
	FLEXT_CALLBACK(m_dir_)
	FLEXT_CALLBACK(m_doc_)

	FLEXT_CALLBACK_V(m_py_float)
	FLEXT_CALLBACK_V(m_py_list)
	FLEXT_CALLBACK_V(m_py_int)
	FLEXT_CALLBACK_A(m_py_any)

#ifdef FLEXT_THREADS
	FLEXT_THREAD_A(work)
#else
	FLEXT_CALLBACK_A(work)
#endif
};

FLEXT_LIB_V("py",pyobj)


void pyobj::Setup(t_classid c)
{
	FLEXT_CADDBANG(c,0,m_bang);
	FLEXT_CADDMETHOD_(c,0,"reload",m_reload_);
	FLEXT_CADDMETHOD_(c,0,"reload.",m_reload);
	FLEXT_CADDMETHOD_(c,0,"set",m_set);
	FLEXT_CADDMETHOD_(c,0,"doc",m_doc);
	FLEXT_CADDMETHOD_(c,0,"doc+",m_doc_);
#ifdef FLEXT_THREADS
	FLEXT_CADDATTR_VAR1(c,"detach",detach);
	FLEXT_CADDMETHOD_(c,0,"stop",m_stop);
#endif
	FLEXT_CADDMETHOD_(c,0,"dir",m_dir);
	FLEXT_CADDMETHOD_(c,0,"dir+",m_dir_);

	FLEXT_CADDMETHOD_(c,1,"float",m_py_float);
	FLEXT_CADDMETHOD_(c,1,"int",m_py_int);
	FLEXT_CADDMETHOD(c,1,m_py_list);
	FLEXT_CADDMETHOD(c,1,m_py_any);

  	FLEXT_CADDATTR_VAR1(c,"respond",respond);
}

pyobj::pyobj(I argc,const t_atom *argv):
	function(NULL),funname(NULL)
{ 
	PY_LOCK

	AddInAnything(2);  
	AddOutAnything();  

	if(argc > 2) 
		SetArgs(argc-2,argv+2);
	else
		SetArgs(0,NULL);

	// init script module
	if(argc >= 1) {
		if(!IsString(argv[0])) 
			post("%s - script name argument is invalid",thisName());
        else {
		    C dir[1024];
		    GetModulePath(GetString(argv[0]),dir,sizeof(dir));
		    // set script path
		    AddToPath(dir);

#if FLEXT_SYS == FLEXT_SYS_PD
			// add dir of current patch to path
			AddToPath(GetString(canvas_getdir(thisCanvas())));
			// add current dir to path
			AddToPath(GetString(canvas_getcurrentdir()));
#elif FLEXT_SYS == FLEXT_SYS_MAX 
			short path = patcher_myvol(thisCanvas());
			path_topathname(path,NULL,dir); 
			AddToPath(dir);       
#else 
	        #pragma message("Adding current dir to path is not implemented")
#endif

			ImportModule(GetString(argv[0]));
        }
	}

	Register("_py");

	if(argc >= 2) {
		// set function name
		if(!IsString(argv[1])) 
			post("%s - function name argument is invalid",thisName());
		else {
			// Set function
			SetFunction(GetString(argv[1]));
		}
	}

	PY_UNLOCK
}

pyobj::~pyobj() 
{
	PY_LOCK
	Unregister("_py");
	PY_UNLOCK
}




BL pyobj::m_method_(I n,const t_symbol *s,I argc,const t_atom *argv)
{
	if(n == 1)
		post("%s - no method for type %s",thisName(),GetString(s));
	return false;
}

V pyobj::m_reload()
{
	PY_LOCK

	Unregister("_py");

	ReloadModule();
	Reregister("_py");
	Register("_py");
	SetFunction(funname?GetString(funname):NULL);

	PY_UNLOCK
}

V pyobj::m_reload_(I argc,const t_atom *argv)
{
	PY_LOCK
	SetArgs(argc,argv);
	PY_UNLOCK

	m_reload();
}

V pyobj::m_set(I argc,const t_atom *argv)
{
	PY_LOCK

	I ix = 0;
	if(argc >= 2) {
		if(!IsString(argv[ix])) {
			post("%s - script name is not valid",thisName());
			return;
		}
		const C *sn = GetString(argv[ix]);

		if(!module || !strcmp(sn,PyModule_GetName(module))) {
			ImportModule(sn);
			Register("_py");
		}

		++ix;
	}

	if(!IsString(argv[ix])) 
		post("%s - function name is not valid",thisName());
	else
		SetFunction(GetString(argv[ix]));

	PY_UNLOCK
}

V pyobj::m_help()
{
	post("");
	post("%s %s - python script object, (C)2002-2004 Thomas Grill",thisName(),PY__VERSION);
#ifdef FLEXT_DEBUG
	post("DEBUG VERSION, compiled on " __DATE__ " " __TIME__);
#endif

	post("Arguments: %s [script name] [function name] {args...}",thisName());

	post("Inlet 1:messages to control the py object");
	post("      2:call python function with message as argument(s)");
	post("Outlet: 1:return values from python function");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\tbang: call script without arguments");
	post("\tset [script name] [function name]: set (script and) function name");
	post("\treload {args...}: reload python script");
	post("\treload. : reload with former arguments");
	post("\tdoc: display module doc string");
	post("\tdoc+: display function doc string");
	post("\tdir: dump module dictionary");
	post("\tdir+: dump function dictionary");
#ifdef FLEXT_THREADS
	post("\tdetach 0/1: detach threads");
	post("\tstop {wait time (ms)}: stop threads");
#endif
	post("");
}

V pyobj::ResetFunction()
{
	if(!module || !dict) 
	{ 
		post("%s - No module loaded",thisName());
		function = NULL; 
		return; 
	}

	function = funname?PyDict_GetItemString(dict,(C *)GetString(funname)):NULL; // borrowed!!!
	if(!function) {
		PyErr_Clear();
		if(funname) post("%s - Function %s could not be found",thisName(),GetString(funname));
	}
	else if(!PyFunction_Check(function)) {
		post("%s - Object %s is not a function",thisName(),GetString(funname));
		function = NULL;
	}
}

V pyobj::SetFunction(const C *func)
{
	if(func) {
		funname = MakeSymbol(func);
		ResetFunction();
	}
	else 
		function = NULL,funname = NULL;
}


V pyobj::Reload()
{
	ResetFunction();
}


BL pyobj::work(const t_symbol *s,I argc,const t_atom *argv)
{
	AtomList *rargs = NULL;
    BL ret;

	++thrcount;
	PY_LOCK

	if(function) {
		PyObject *pArgs = MakePyArgs(s,argc,argv);
		PyObject *pValue = PyObject_CallObject(function, pArgs);

		rargs = GetPyArgs(pValue);
		if(!rargs) PyErr_Print();

		Py_XDECREF(pArgs);
		Py_XDECREF(pValue);
        ret = true;
	}
	else {
		post("%s: no function defined",thisName());
        ret = false;
	}

	PY_UNLOCK
	--thrcount;

	if(rargs) {
		// call to outlet _outside_ the Mutex lock!
		// otherwise (if not detached) deadlock will occur
		ToOutList(0,*rargs);
		delete rargs;
	}

    return ret;
}

V pyobj::callwork(const t_symbol *s,I argc,const t_atom *argv)
{
    BL ret = false;
	if(detach) {
		if(shouldexit)
			post("%s - New threads can't be launched now!",thisName());
		else {
			ret = FLEXT_CALLMETHOD_A(work,s,argc,argv);
			if(!ret) post("%s - Failed to launch thread!",thisName());
        }
	}
    else
		ret = work(s,argc,argv);
    Respond(ret);
}


