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
	~pyext();

	static PyObject *py__init__(PyObject *self,PyObject *args);
	static PyObject *py_outlet(PyObject *self,PyObject *args);

protected:
	BL m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	virtual BL work(I n,const t_symbol *s,I argc,t_atom *argv); 

//	virtual V m_bang() { work(sym_bang,0,NULL); }
	virtual V m_reload(I argc,t_atom *argv);
	virtual V m_set(I argc,t_atom *argv);

	virtual V m_help();

	PyObject *pyobj;

private:
	enum retval { nothing,atom,tuple,list };

	static V setup(t_class *);
	
	BL call(const C *meth,const t_symbol *s,I argc,t_atom *argv);
	
//	FLEXT_CALLBACK(m_bang)

	FLEXT_CALLBACK_G(m_reload)
	FLEXT_CALLBACK_G(m_set)
};

FLEXT_LIB_G("pyext",pyext)



PyObject* pyext::py__init__(PyObject *self, PyObject *args)
{
    post("pyext.__init__ called");
    Py_INCREF(Py_None);
    return Py_None;
}


PyObject *pyext::py_outlet(PyObject *self,PyObject *args) 
{
    post("pyext.outlet called");
	if(self) {
		PyObject *pmod  = PyObject_GetAttrString(self,"__module__"); // get module
		if(pmod) {
			Py_DECREF(pmod);
		}
		else 
			PyErr_Print();
		
		I rargc;
		t_atom *rargv = GetPyArgs(rargc,args);
	/*
		pyext *ext = NULL;

		if(rargv && rargc >= 1) {
			I o = GetAInt(rargv[0]);
			if(rargc >= 2 && IsSymbol(rargv[1]))
				ext->ToOutAnything(o,GetSymbol(rargv[1]),rargc-2,rargv+2);
			else
				ext->ToOutList(o,rargc-1,rargv+1);
		}
		else {
			PyErr_Print();
	//			post("%s: python function call failed",thisName());
		}
	*/
		if(rargv) delete[] rargv;
	}

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef pyext_meths[] = 
{
    {"__init__", pyext::py__init__, METH_VARARGS, "pyext init"},
    {"outlet", pyext::py_outlet, METH_VARARGS,"pyext outlet"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyMethodDef mod_meths[] = {{NULL, NULL, 0, NULL}};



pyext::pyext(I argc,t_atom *argv):
	pyobj(NULL)
{ 
	if(pyref == 1) {
		PyObject *module = Py_InitModule("pyext", pyext_meths);
/*
		PyObject *module = Py_InitModule("pyext", mod_meths);

		PyObject *moduleDict = PyModule_GetDict(module);
		PyObject *classDict = PyDict_New();
		PyObject *className = PyString_FromString("pyext");
		PyObject *fooClass = PyClass_New(NULL, classDict, className);
		PyDict_SetItemString(moduleDict, "pyext", fooClass);
		Py_DECREF(classDict);
		Py_DECREF(className);
		Py_DECREF(fooClass);
    
		// add methods to class 
		for (PyMethodDef *def = pyext_meths; def->ml_name != NULL; def++) {
			PyObject *func = PyCFunction_New(def, NULL);
			PyObject *method = PyMethod_New(func, NULL, fooClass);
			PyDict_SetItemString(classDict, def->ml_name, method);
			Py_DECREF(func);
			Py_DECREF(method);
		}
*/
	}
		
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

	t_symbol *sobj = NULL;
	if(argc >= 2) {
		// object name
		if(!IsString(argv[1])) 
			post("%s - object name argument is invalid",thisName());
		else {
			sobj = GetSymbol(argv[1]);
		}
	}

	if(sobj) {
//		if(argc > 2) SetArgs(argc-2,argv+2);

		PyObject *pmod = GetModule();
		
		if(pmod) {
			PyObject *pclass = PyObject_GetAttrString(pmod,const_cast<C *>(GetString(sobj)));   /* fetch module.class */
			Py_DECREF(pmod);
			if (pclass == NULL) 
				PyErr_Print();
				//error("Can't instantiate class");

			PyObject *pargs = MakePyArgs(NULL,argc-2,argv+2);
			if (pargs == NULL) 
				PyErr_Print();
	//			error("Can't build arguments list");

			if(pclass) {
				pyobj = PyEval_CallObject(pclass, pargs);         /* call class() */
				Py_DECREF(pclass);
				if(pargs) Py_DECREF(pargs);
				if(pyobj == NULL) 
					PyErr_Print();
			}
		}
	}
		
	AddInAnything(2);  
	AddOutAnything();  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDMETHOD_(0,"reload",m_reload);
	FLEXT_ADDMETHOD_(0,"set",m_set);

//	if(!pyobj) InitProblem();
}

pyext::~pyext()
{
	if(pyobj) Py_DECREF(pyobj);
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


BL pyext::m_method_(I n,const t_symbol *s,I argc,t_atom *argv)
{
	if(pyobj && n >= 1) {
		return work(n,s,argc,argv);
	}
	else {
		post("%s - no method for type '%s' into inlet %i",thisName(),GetString(s),n);
		return false;
	}
}


V pyext::m_help()
{
	post("pyext %s - python script object, (C)2002 Thomas Grill",PY__VERSION);
#ifdef _DEBUG
	post("compiled on " __DATE__ " " __TIME__);
#endif

	post("Arguments: %s [script name] [object name] [args...]",thisName());

	post("Inlet 1: messages to control the pyext object");
	post("      2...: python inlets");
	post("Outlet: 1: return values from python function");	
	post("        2...: python outlets");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\tbang: call script without arguments");
	post("\tset [script name] [object name]: set (script and) object name");
	post("\treload [args...]: reload python script");
	post("");
}

BL pyext::call(const C *meth,const t_symbol *s,I argc,t_atom *argv) 
{
	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
		return false;
	}
	else {
		PyObject *pargs = MakePyArgs(s,argc,argv);
		if(!pargs)
			PyErr_Print();
		else {
			PyObject *pres = PyEval_CallObject(pmeth, pargs);           /* call method(x,y) */
			if (pres == NULL)
				PyErr_Print();
			else {
				Py_DECREF(pres);
			}

			Py_DECREF(pargs);
		}
		Py_DECREF(pmeth);

		return true;
	}
}

BL pyext::work(I n,const t_symbol *s,I argc,t_atom *argv)
{
	BL ret = false;
	char *str = new char[strlen(GetString(s))+10];

	sprintf(str,"_%s_%i",GetString(s),n);
	ret = call(str,s,argc,argv);
	if(!ret) {
		sprintf(str,"_anything_%i",n);
		ret = call(str,s,argc,argv);
	}
	if(!ret) {
		sprintf(str,"_%s_",GetString(s));
		ret = call(str,s,argc,argv);
	}
	if(!ret) {
		strcpy(str,"_anything_");
		ret = call(str,s,argc,argv);
	}
	if(!ret) post("%s - no matching method found for '%s' into inlet %i",thisName(),GetString(s),n);

	if(str) delete[] str;

	return ret;
}



