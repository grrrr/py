/* 

pyext - python external object for PD and MaxMSP

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

	static PyObject *py_new(PyObject *self,PyObject *args);
	static PyObject *py__init__(PyObject *self,PyObject *args);
	static PyObject *py__getitem__(PyObject *self,PyObject *args);
	static PyObject *py_outlet(PyObject *self,PyObject *args);
	static PyObject *py_setattr(PyObject *self,PyObject *args);
	static PyObject *py_getattr(PyObject *self,PyObject *args);

	I Inlets() const { return inlets; }
	I Outlets() const { return outlets; }

protected:
	BL m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	BL work(I n,const t_symbol *s,I argc,t_atom *argv); 

	V m_reload(I argc,t_atom *argv);
	virtual V m_help();

	PyObject *pyobj;
	I inlets,outlets;

private:
	enum retval { nothing,atom,tuple,list };

	static V setup(t_class *);
	
	PyObject *call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv);
	PyObject *call(const C *meth);

	FLEXT_CALLBACK_V(m_reload)
	FLEXT_CALLBACK_B(m_detach)
	FLEXT_CALLBACK_I(m_wait)

	V work_wrapper(void *data); 
	BL callwork(I n,const t_symbol *s,I argc,t_atom *argv); 

	class work_data:
		public flext_base::AtomAnything
	{
	public:
		work_data(I _n,const t_symbol *_s,I _argc,t_atom *_argv): n(_n),AtomAnything(_s,_argc,_argv) {}
		I n;
	};

#ifdef FLEXT_THREADS
	FLEXT_THREAD_X(work_wrapper)
#else
	FLEXT_CALLBACK_X(work_wrapper)
#endif

	PyThreadState *pythr;
};

FLEXT_LIB_V("pyext",pyext)


PyObject* pyext::py__init__(PyObject *sl, PyObject *args)
{
//    post("pyext.__init__ called");

    Py_INCREF(Py_None);
    return Py_None;
}


PyObject* pyext::py_setattr(PyObject *sl, PyObject *args)
{
    PyObject *selfobj, *arg0,*arg1;
    if(!PyArg_ParseTuple(args, "OOO:test_foo", &selfobj, &arg0,&arg1)) {
        // handle error
		error("pyext - parsetuple failed");
		return NULL;
    }
    if (!PyString_Check(arg0)) {
        return NULL;
    }    
    char* name = PyString_AsString(arg0);
    if (!name) return NULL;

	post("pyext::setattr %s",name);

/*
    if (strcmp(name, "id")==0) {
        return Py_BuildValue("i", 42);
    }
*/
    // throw attribute not found and and ...

/*
    Py_INCREF(Py_None);
    return Py_None;
*/
	return NULL;
}

PyObject* pyext::py_getattr(PyObject *sl, PyObject *args)
{
    PyObject *selfobj, *arg0;
    if(!PyArg_ParseTuple(args, "OO:test_foo", &selfobj, &arg0)) {
        // handle error
		error("pyext - parsetuple failed");
    }
    if (!PyString_Check(arg0)) {
        return NULL;
    }    
    char* name = PyString_AsString(arg0);
    if (!name) return NULL;

	post("pyext::getattr %s",name);
/*
    if (strcmp(name, "id")==0) {
        return Py_BuildValue("i", 42);
    }
*/
    // throw attribute not found and and ...

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::py__getitem__(PyObject *sl, PyObject *args)
{
    PyObject *selfobj, *arg0;
    if(!PyArg_ParseTuple(args, "OO:test_foo", &selfobj, &arg0)) {
        // handle error
		error("pyext - parsetuple failed");
    }
    if (!PyString_Check(arg0)) {
        return NULL;
    }    
    char* name = PyString_AsString(arg0);
    if (!name) return NULL;

	post("pyext::getitem %s",name);
/*
    if (strcmp(name, "id")==0) {
        return Py_BuildValue("i", 42);
    }
*/
    // throw attribute not found and and ...

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *pyext::py_outlet(PyObject *sl,PyObject *args) 
{
//	py_BEGIN_ALLOW_THREADS

	I rargc;
	PyObject *self;
	t_atom *rargv = GetPyArgs(rargc,args,&self);

//    post("pyext.outlet called, args:%i",rargc);

	if(rargv) {
		pyext *ext = NULL;
		PyObject *th = PyObject_GetAttrString(self,"thisptr");
		if(th) {
			ext = (pyext *)PyLong_AsVoidPtr(th); 
		}
		else {
			post("pyext - internal error " __FILE__ "/%i",__LINE__);
		}

		if(ext && rargv && rargc >= 2) {
			I o = GetAInt(rargv[1]);
			if(o >= 1 && o <= ext->Outlets()) {
				if(rargc >= 3 && IsSymbol(rargv[2]))
					ext->ToOutAnything(o-1,GetSymbol(rargv[2]),rargc-3,rargv+3);
				else
					ext->ToOutList(o-1,rargc-2,rargv+2);
			}
			else
				post("pyext: outlet index out of range");
		}
		else {
//			PyErr_Print();
			post("pyext: python function call failed");
		}
	}

	if(rargv) delete[] rargv;

    Py_INCREF(Py_None);

//	py_END_ALLOW_THREADS
    return Py_None;
}


static PyMethodDef pyext_meths[] = 
{
//    {"new", pyext::py_new, METH_VARARGS, "pyext new"},
    {"__init__", pyext::py__init__, METH_VARARGS, "pyext init"},
//    {"__getitem__", pyext::py__getitem__, METH_VARARGS, "pyext getitem"},
    {"_outlet", pyext::py_outlet, METH_VARARGS,"pyext outlet"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyMethodDef pyext_attrs[] =
{
//	{ "__setattr__", pyext::py_setattr, METH_VARARGS,"pyext setattr" },
//	{ "__getattr__", pyext::py_getattr, METH_VARARGS,"pyext getattr" },
  { NULL, NULL,0,NULL },
};



static PyMethodDef pyext_mod_meths[] = {{NULL, NULL, 0, NULL}};

static I ref = 0;

pyext::pyext(I argc,t_atom *argv):
	pyobj(NULL),pythr(NULL),
	inlets(0),outlets(0)
{ 
//	py_BEGIN_ALLOW_THREADS

	if(!(ref++)) {
		PyObject *module = Py_InitModule("pyext", pyext_mod_meths);

		PyObject *moduleDict = PyModule_GetDict(module);
		PyObject *classDict = PyDict_New();
		PyObject *className = PyString_FromString("pybase");
/*
		// add setattr/getattr to class 
		for(PyMethodDef* _def = pyext_attrs; _def->ml_name; _def++) {
			  PyObject *func = PyCFunction_New(_def, NULL);
			  PyDict_SetItemString(classDict, _def->ml_name, func);
			  Py_DECREF(func);
		}
*/

//		PyDict_SetItemString(moduleDict, "__builtins__",PyEval_GetBuiltins());
//		PyDict_SetItemString(classDict, "__builtins__",PyEval_GetBuiltins());

		PyObject *fooClass = PyClass_New(NULL, classDict, className);
		PyDict_SetItemString(moduleDict, "pybase", fooClass);
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
 	}

	// init script module
	if(argc >= 1) {
		C dir[1024];
		GetModulePath(GetString(argv[0]),dir,sizeof(dir));
		// add to path
		AddToPath(dir);

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
		if(argc > 2) 
			SetArgs(argc-2,argv+2);
		else
			SetArgs(0,NULL);

		PyObject *pmod = GetModule();
		
		if(pmod) {
			PyObject *pclass = PyObject_GetAttrString(pmod,const_cast<C *>(GetString(sobj)));   /* fetch module.class */
			Py_DECREF(pmod);
			if (!pclass) 
				PyErr_Print();
			else {
				PyObject *pargs = MakePyArgs(NULL,argc-2,argv+2);
				if (pargs == NULL) PyErr_Print();

				pyobj = PyEval_CallObject(pclass, pargs);         /* call class() */
				Py_DECREF(pclass);
				if(pargs) Py_DECREF(pargs);
				if(pyobj == NULL) 
					PyErr_Print();
				else {
					PyObject *th = PyLong_FromVoidPtr(this); 
					int ret = PyObject_SetAttrString(pyobj,"thisptr",th);

					PyObject *res;
					res = call("_inlets");
					if(res) {
						inlets = PyInt_AsLong(res);
						Py_DECREF(res);
					}
					else inlets = 1;
					res = call("_outlets");
					if(res) {
						outlets = PyInt_AsLong(res);
						Py_DECREF(res);
					}
					else outlets = 1;
				}
			}
		}
	}
		
	AddInAnything(1+inlets);  
	AddOutAnything(outlets);  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDMETHOD_(0,"reload",m_reload);
	FLEXT_ADDMETHOD_(0,"detach",m_detach);
	FLEXT_ADDMETHOD_(0,"wait",m_wait);

//	py_END_ALLOW_THREADS
}

pyext::~pyext()
{
	--ref;
	if(pyobj) Py_DECREF(pyobj);
}

V pyext::m_reload(I argc,t_atom *argv)
{
	if(argc > 2) SetArgs(argc,argv);

	ReloadModule();
}


BL pyext::m_method_(I n,const t_symbol *s,I argc,t_atom *argv)
{
	if(pyobj && n >= 1) {
		return callwork(n,s,argc,argv);
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
	post("Outlets: python outlets");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\treload [args...]: reload python script");
	post("\tdetach 0/1: detach threads");
	post("\twait [int]: wait time for thread termination (in ms)");
	post("");
}

PyObject *pyext::call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv) 
{
//	py_BEGIN_ALLOW_THREADS

	PyObject *ret = NULL;
	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
	}
	else {
		PyObject *pargs = MakePyArgs(s,argc,argv,inlet?inlet:-1);
		if(!pargs)
			PyErr_Print();
		else {
			ret = PyEval_CallObject(pmeth, pargs);           /* call method(x,y) */
			if (ret == NULL)
				PyErr_Print();
			else {
//				Py_DECREF(pres);
			}

			Py_DECREF(pargs);
		}
		Py_DECREF(pmeth);
	}

//	py_END_ALLOW_THREADS

	return ret;
}

PyObject *pyext::call(const C *meth) 
{
//	py_BEGIN_ALLOW_THREADS

	PyObject *pres = NULL;
	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
	}
	else {
//		PyObject *sf = PyTuple_New(0);
//		PyTuple_SetItem(sf,0,pyobj); 

		pres = PyEval_CallObject(pmeth, NULL);           /* call method(x,y) */
		if (pres == NULL)
			PyErr_Print();
		else {
//			Py_DECREF(pres);
		}
		Py_DECREF(pmeth);
//		Py_DECREF(sf);
	}

//	py_END_ALLOW_THREADS

	return pres;
}

V pyext::work_wrapper(V *data)
{
	work_data *w = (work_data *)data;
	work(w->n,w->Header(),w->Count(),w->Atoms());
	delete w;
}

BL pyext::callwork(I n,const t_symbol *s,I argc,t_atom *argv)
{
	if(detach) {
		FLEXT_CALLMETHOD_X(work_wrapper,new work_data(n,s,argc,argv));
		return true;
	}
	else 
		return work(n,s,argc,argv);
}

BL pyext::work(I n,const t_symbol *s,I argc,t_atom *argv)
{
	BL retv = false;

	PyObject *ret = NULL;
	char *str = new char[strlen(GetString(s))+10];

	{
		// try tag/inlet
		sprintf(str,"_%s_%i",GetString(s),n);
		ret = call(str,0,NULL,argc,argv);
	}

	if(!ret) {
		// try anything/inlet
		sprintf(str,"_anything_%i",n);
		if(s == sym_bang && !argc) {
			t_atom argv;
			SetString(argv,"");
			ret = call(str,0,s,1,&argv);
		}
		else
			ret = call(str,0,s,argc,argv);
	}
	if(!ret) {
		// try tag at any inlet
		sprintf(str,"_%s_",GetString(s));
		ret = call(str,n,NULL,argc,argv);
	}
	if(!ret) {
		// try anything at any inlet
		strcpy(str,"_anything_");
		if(s == sym_bang && !argc) {
			t_atom argv;
			SetString(argv,"");
			ret = call(str,n,s,1,&argv);
		}
		else
			ret = call(str,n,s,argc,argv);
	}

	if(!ret) 
		// no matching python method found
		post("%s - no matching method found for '%s' into inlet %i",thisName(),GetString(s),n);

	if(str) delete[] str;

	if(ret) {
//		Py_BEGIN_ALLOW_THREADS
		if(!PyObject_Not(ret)) post("%s - returned value is ignored",thisName());
		Py_DECREF(ret);
		retv = true;
//		Py_END_ALLOW_THREADS
	}

	return retv;
}



