/* 

pyext - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#include "main.h"

class pyext:
	public py
{
	FLEXT_HEADER(pyext,py)

public:
	pyext(I argc,t_atom *argv);

protected:
	V m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	V work(const t_symbol *s,I argc,t_atom *argv); 

	virtual V m_bang() { work(sym_bang,0,NULL); }
	virtual V m_reload(I argc,t_atom *argv);
	virtual V m_set(I argc,t_atom *argv);

	virtual V m_help();

	// methods for python arguments
	virtual V m_py_list(I argc,t_atom *argv) { work(sym_list,argc,argv); }
	virtual V m_py_float(I argc,t_atom *argv) { work(sym_float,argc,argv); }
	virtual V m_py_int(I argc,t_atom *argv) { work(sym_int,argc,argv); }
	virtual V m_py_any(const t_symbol *s,I argc,t_atom *argv) { work(s,argc,argv); }

	C *sFunc;

private:
	enum retval { nothing,atom,tuple,list };

	static V setup(t_class *);
	
	FLEXT_CALLBACK(m_bang)

	FLEXT_CALLBACK_G(m_reload)
	FLEXT_CALLBACK_G(m_set)
	FLEXT_CALLBACK_G(m_py_float)
	FLEXT_CALLBACK_G(m_py_list)
	FLEXT_CALLBACK_G(m_py_int)
	FLEXT_CALLBACK_A(m_py_any)
};

FLEXT_LIB_G("pyext",pyext)


pyext::pyext(I argc,t_atom *argv):
	sFunc(NULL)
{ 
	AddInAnything(2);  
	AddOutAnything();  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDBANG(0,m_bang);
	FLEXT_ADDMETHOD_(0,"reload",m_reload);
	FLEXT_ADDMETHOD_(0,"set",m_set);

	FLEXT_ADDMETHOD_(1,"float",m_py_float);
	FLEXT_ADDMETHOD_(1,"int",m_py_int);
	FLEXT_ADDMETHOD(1,m_py_list);
	FLEXT_ADDMETHOD(1,m_py_any);

	if(argc > 2) SetArgs(argc-2,argv+2);

	// init script module
	if(argc >= 1) {
		C dir[1024];
#ifdef PD
		// uarghh... pd doesn't show it's path for extra modules

		C *name;
		I fd = open_via_path("",GetString(argv[0]),".py",dir,&name,sizeof(dir),0);
		if(fd > 0) close(fd);
		else name = NULL;
#elif defined(MAXMSP)
		*dir = 0;
#endif

		// set script path
		PySys_SetPath(dir);

		if(!IsString(argv[0])) 
			post("%s - script name argument is invalid",thisName());
		else
			ImportModule(GetString(argv[0]));
	}

	if(argc >= 2) {
		// set function name
		if(!IsString(argv[1])) 
			post("%s - function name argument is invalid",thisName());
//		else
//			SetFunction(GetString(argv[1]));
	}
}


V pyext::m_reload(I argc,t_atom *argv)
{
	if(argc > 2) SetArgs(argc,argv);

	ReloadModule();
}

V pyext::m_set(I argc,t_atom *argv)
{
	I ix = 0;
	if(argc >= 2) {
		if(!IsString(argv[ix])) {
			post("%s - script name is not valid",thisName());
			return;
		}
		const C *sn = GetString(argv[ix]);
		if(strcmp(sn,sName)) ImportModule(sn);
		++ix;
	}

	if(!IsString(argv[ix])) 
		post("%s - function name is not valid",thisName());
//	else
//		SetFunction(GetString(argv[ix]));

}


V pyext::m_method_(I n,const t_symbol *s,I argc,t_atom *argv)
{
	if(n == 1)
		post("%s - no method for type %s",thisName(),GetString(s));
}


V pyext::m_help()
{
	post("pyext %s - python script object, (C)2002 Thomas Grill",PY__VERSION);
#ifdef _DEBUG
	post("compiled on " __DATE__ " " __TIME__);
#endif

	post("Arguments: %s [script name] [function name] [args...]",thisName());

	post("Inlet 1:messages to control the pyext object");
	post("      2:call python function with message as argument(s)");
	post("Outlet: 1:return values from python function");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\tbang: call script without arguments");
	post("\tset [script name] [function name]: set (script and) function name");
	post("\treload [args...]: reload python script");
	post("");
}

V pyext::work(const t_symbol *s,I argc,t_atom *argv)
{
	PyObject *pFunc = GetFunction(sFunc);

	if(pFunc && PyCallable_Check(pFunc)) {
		PyObject *pArgs = MakeArgs(s,argc,argv);
		PyObject *pValue = PyObject_CallObject(pFunc, pArgs);

		I rargc;
		t_atom *rargv = GetRets(rargc,pValue);

		if(rargv) {
			ToOutList(0,rargc,rargv);
			delete[] rargv;
		}
		else {
			PyErr_Print();
//			post("%s: python function call failed",thisName());
		}
		if(pArgs) Py_DECREF(pArgs);
		if(pValue) Py_DECREF(pValue);
	}
	else {
		post("%s: no function defined",thisName());
	}
}



