/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


class pyobj
    : public pybase
    , public flext_base
{
	FLEXT_HEADER_S(pyobj,flext_base,Setup)

public:
	pyobj(int argc,const t_atom *argv);
	~pyobj();

protected:
    virtual void Exit();

	virtual bool CbMethodResort(int n,const t_symbol *s,int argc,const t_atom *argv);
    virtual void CbClick();

    void m_help();    

	void m_reload();
	void m_reload_(int argc,const t_atom *argv);
	void m_set(int argc,const t_atom *argv);
    void m_dir_() { m__dir(function); }
    void m_doc_() { m__doc(function); }

	// methods for python arguments
	void callwork(const t_symbol *s,int argc,const t_atom *argv);
	
	inline void m_bang() { callwork(NULL,0,NULL); }
	inline void m_py_list(int argc,const t_atom *argv) { callwork(sym_list,argc,argv); }
	inline void m_py_float(int argc,const t_atom *argv) { callwork(sym_float,argc,argv); }
	inline void m_py_int(int argc,const t_atom *argv) { callwork(sym_int,argc,argv); }
	inline void m_py_any(const t_symbol *s,int argc,const t_atom *argv) { callwork(s,argc,argv); }

	const t_symbol *funname;
	PyObject *function;
    bool withfunction;

	virtual void Reload();

	void SetFunction(const char *func);
	void ResetFunction();

    virtual bool thrcall(void *data);
    virtual void DumpOut(const t_symbol *sym,int argc,const t_atom *argv);

private:

    virtual bool callpy(PyObject *fun,PyObject *args);

	static void Setup(t_classid c);

	FLEXT_CALLBACK(m_help)
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

	// callbacks
	FLEXT_ATTRVAR_I(detach)
	FLEXT_ATTRVAR_B(respond)
	FLEXT_CALLBACK_V(m_stop)
	FLEXT_CALLBACK(m_dir)
	FLEXT_CALLGET_V(mg_dir)
	FLEXT_CALLBACK(m_doc)

#ifdef FLEXT_THREADS
    FLEXT_CALLBACK_T(tick)
    FLEXT_THREAD(threadworker)
	FLEXT_THREAD_X(work_wrapper)
#else
	FLEXT_CALLBACK_X(work_wrapper)
#endif
};

FLEXT_LIB_V("py",pyobj)


void pyobj::Setup(t_classid c)
{
	FLEXT_CADDMETHOD_(c,0,"doc",m_doc);
	FLEXT_CADDMETHOD_(c,0,"dir",m_dir);
#ifdef FLEXT_THREADS
	FLEXT_CADDATTR_VAR1(c,"detach",detach);
	FLEXT_CADDMETHOD_(c,0,"stop",m_stop);
#endif

	FLEXT_CADDMETHOD_(c,0,"help",m_help);
	FLEXT_CADDMETHOD_(c,0,"reload",m_reload_);
    FLEXT_CADDMETHOD_(c,0,"reload.",m_reload);
	FLEXT_CADDMETHOD_(c,0,"doc+",m_doc_);
	FLEXT_CADDMETHOD_(c,0,"dir+",m_dir_);

	FLEXT_CADDBANG(c,0,m_bang);
	FLEXT_CADDMETHOD_(c,0,"set",m_set);

	FLEXT_CADDMETHOD_(c,1,"float",m_py_float);
	FLEXT_CADDMETHOD_(c,1,"int",m_py_int);
	FLEXT_CADDMETHOD(c,1,m_py_list);
	FLEXT_CADDMETHOD(c,1,m_py_any);

  	FLEXT_CADDATTR_VAR1(c,"respond",respond);
}

pyobj::pyobj(int argc,const t_atom *argv):
	function(NULL),funname(NULL),withfunction(false)
{ 
	AddInAnything(2);  
	AddOutAnything();  

#ifdef FLEXT_THREADS
    FLEXT_ADDTIMER(stoptmr,tick);
    // launch thread worker
    FLEXT_CALLMETHOD(threadworker);
#endif

	PyThreadState *state = PyLockSys();

	if(argc > 2) 
		SetArgs(argc-2,argv+2);
	else
		SetArgs(0,NULL);

	// init script module
	if(argc >= 1) {
		if(!IsString(argv[0])) 
			post("%s - script name argument is invalid",thisName());
        else {
		    char dir[1024];
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
        withfunction = true;

		// set function name
		if(!IsString(argv[1])) 
			post("%s - function name argument is invalid",thisName());
		else {
			// Set function
			SetFunction(GetString(argv[1]));
		}
	}
    else
        withfunction = false;

	PyUnlock(state);
}

pyobj::~pyobj() 
{
	PyThreadState *state = PyLockSys();
	Unregister("_py");
	PyUnlock(state);
}

void pyobj::Exit() 
{ 
    pybase::Exit(); 
    flext_base::Exit(); 
}

bool pyobj::CbMethodResort(int n,const t_symbol *s,int argc,const t_atom *argv)
{
	if(n == 1)
		post("%s - no method for type %s",thisName(),GetString(s));
	return false;
}

void pyobj::m_reload()
{
	PyThreadState *state = PyLockSys();

	Unregister("_py");

	ReloadModule();
	Reregister("_py");
	Register("_py");
	SetFunction(funname?GetString(funname):NULL);

	PyUnlock(state);
}

void pyobj::m_reload_(int argc,const t_atom *argv)
{
	PyThreadState *state = PyLockSys();
	SetArgs(argc,argv);
	PyUnlock(state);

	m_reload();
}

void pyobj::m_set(int argc,const t_atom *argv)
{
	PyThreadState *state = PyLockSys();

	int ix = 0;
	if(argc >= 2) {
		if(!IsString(argv[ix])) {
			post("%s - script name is not valid",thisName());
			return;
		}
		const char *sn = GetString(argv[ix]);

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

	PyUnlock(state);
}

void pyobj::m_help()
{
	post("");
	post("%s %s - python script object, (C)2002-2005 Thomas Grill",thisName(),PY__VERSION);
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
	post("\tdetach 0/1/2: detach threads");
	post("\tstop {wait time (ms)}: stop threads");
#endif
	post("");
}

void pyobj::ResetFunction()
{
	if(!module || !dict) 
	{ 
		post("%s - No module loaded",thisName());
		function = NULL; 
		return; 
	}

	function = funname?PyDict_GetItemString(dict,(char *)GetString(funname)):NULL; // borrowed!!!
	if(!function) {
		PyErr_Clear();
		if(funname) post("%s - Function %s could not be found",thisName(),GetString(funname));
	}
	else if(!PyFunction_Check(function)) {
		post("%s - Object %s is not a function",thisName(),GetString(funname));
		function = NULL;
	}
}

void pyobj::SetFunction(const char *func)
{
	if(func) {
		funname = MakeSymbol(func);
        withfunction = true;
		ResetFunction();
	}
    else {
        withfunction = false;
		function = NULL,funname = NULL;
    }
}


void pyobj::Reload()
{
	ResetFunction();
}

bool pyobj::callpy(PyObject *fun,PyObject *args)
{
    PyObject *ret = PyObject_CallObject(fun,args); 
    if(ret == NULL) {
        // function not found resp. arguments not matching
        PyErr_Print();
        return false;
    }
    else {
        AtomList *rargs = GetPyArgs(ret);
		if(!rargs) 
            PyErr_Print();
        else {
            // call to outlet _outside_ the Mutex lock!
            // otherwise (if not detached) deadlock will occur
            if(rargs->Count()) ToOutList(0,*rargs);
            delete rargs;
        }
        Py_DECREF(ret);
        return true;
    }
} 

void pyobj::callwork(const t_symbol *s,int argc,const t_atom *argv)
{
    bool ret = false;
 
    PyThreadState *state = PyLock();

    if(withfunction) {
        if(function) {
		    PyObject *pargs = MakePyArgs(s,argc,argv);
            Py_INCREF(function);
            ret = gencall(function,pargs);
        }
	    else
		    post("%s: no valid function defined",thisName());
    }
    else if(module) {
        // no function defined as creation argument -> use message tag
        PyObject *func = PyObject_GetAttrString(module,const_cast<char *>(GetString(s)));
        if(func) {
		    PyObject *pargs = MakePyArgs(sym_list,argc,argv);
            ret = gencall(func,pargs);
        }
        else
            PyErr_Print();
    }

    PyUnlock(state);

    Respond(ret);
}

void pyobj::CbClick() { pybase::OpenEditor(); }

void pyobj::DumpOut(const t_symbol *sym,int argc,const t_atom *argv)
{
    ToOutAnything(GetOutAttr(),sym?sym:thisTag(),argc,argv);
}

bool pyobj::thrcall(void *data)
{ 
    return FLEXT_CALLMETHOD_X(work_wrapper,data);
}
