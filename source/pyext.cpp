/* 

py/pyext - python external object for PD and MaxMSP

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
	static PyObject *class_obj,*class_dict;
	static PyMethodDef attr_tbl[],meth_tbl[];

	PyObject *call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv);

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

private:
	FLEXT_CALLBACK_V(m_reload)
};

FLEXT_LIB_V("pyext",pyext)


PyObject* pyext::py__init__(PyObject *,PyObject *args)
{
//    post("pyext.__init__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::py_setattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*val,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OOO:test_foo", &self,&name,&val)) {
        // handle error
		error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
		return NULL;
    }

	BL handled = false;
    if(PyString_Check(name)) {
	    char* sname = PyString_AsString(name);
		if (sname) {
//			post("pyext::setattr %s",sname);
		}
	}

	if(!handled) {
		if(PyInstance_Check(self)) 
			PyDict_SetItem(((PyInstanceObject *)self)->in_dict, name,val);
		else
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* pyext::py_getattr(PyObject *,PyObject *args)
{
    PyObject *self,*name,*ret = NULL;
    if(!PyArg_ParseTuple(args, "OO:test_foo", &self,&name)) {
        // handle error
		error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }

    if(PyString_Check(name)) {
	    char* sname = PyString_AsString(name);
		if (sname) {
			if(!strcmp(sname,"_shouldexit")) {
				PyObject *th = PyObject_GetAttrString(self,"_this");
				if(th) {
					pyext *ext = (pyext *)PyLong_AsVoidPtr(th); 
					ret = PyLong_FromLong(ext->shouldexit?1:0);
				}
			}
//			post("pyext::getattr %s",sname);
		}
	}

	return ret?ret:PyObject_GenericGetAttr(self,name);
}

PyObject *pyext::py_outlet(PyObject *,PyObject *args) 
{
//	PY_LOCK

	I rargc;
	PyObject *self;
	t_atom *rargv = GetPyArgs(rargc,args,&self);

//    post("pyext.outlet called, args:%i",rargc);

	if(rargv) {
		pyext *ext = NULL;
		PyObject *th = PyObject_GetAttrString(self,"_this");
		if(th) {
			ext = (pyext *)PyLong_AsVoidPtr(th); 
		}
		else {
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
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
			error("pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
		}
	}

	if(rargv) delete[] rargv;

    Py_INCREF(Py_None);

//	PY_UNLOCK
    return Py_None;
}


PyMethodDef pyext::meth_tbl[] = 
{
//    {"__init__", pyext::py__init__, METH_VARARGS, "pyext init"},
    {"_outlet", pyext::py_outlet, METH_VARARGS,"pyext outlet"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMethodDef pyext::attr_tbl[] =
{
	{ "__setattr__", pyext::py_setattr, METH_VARARGS,"pyext setattr" },
	{ "__getattr__", pyext::py_getattr, METH_VARARGS,"pyext getattr" },
	{ NULL, NULL,0,NULL },
};


PyObject *pyext::class_obj = NULL;
PyObject *pyext::class_dict = NULL;

pyext::pyext(I argc,t_atom *argv):
	pyobj(NULL),pythr(NULL),
	inlets(0),outlets(0)
{ 
	PY_LOCK

	if(pyref == 1) {
		// register/initialize pyext base class along with module
		class_dict = PyDict_New();
		PyObject *className = PyString_FromString(PYEXT_CLASS);
		PyMethodDef *def;

		// add setattr/getattr to class 
		for(def = attr_tbl; def->ml_name; def++) {
			  PyObject *func = PyCFunction_New(def, NULL);
			  PyDict_SetItemString(class_dict, def->ml_name, func);
			  Py_DECREF(func);
		}

		class_obj = PyClass_New(NULL, class_dict, className);
		PyDict_SetItemString(module_dict, PYEXT_CLASS,class_obj);
		Py_DECREF(className);
    
		// add methods to class 
		for (def = meth_tbl; def->ml_name != NULL; def++) {
			PyObject *func = PyCFunction_New(def, NULL);
			PyObject *method = PyMethod_New(func, NULL, class_obj);
			PyDict_SetItemString(class_dict, def->ml_name, method);
			Py_DECREF(func);
			Py_DECREF(method);
		}
 	}
	else {
		Py_INCREF(class_obj);
		Py_INCREF(class_dict);
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

		if(module) {
			PyObject *pref = PyObject_GetAttrString(module,const_cast<C *>(GetString(sobj)));  
			if (!pref) 
				PyErr_Print();
			else if(PyClass_Check(pref)) {
				PyObject *pargs = MakePyArgs(NULL,argc-2,argv+2);
				if (pargs == NULL) PyErr_Print();

				// call class
				pyobj = PyInstance_New(pref, pargs,NULL);
				Py_DECREF(pref);
				Py_XDECREF(pargs);
				if(pyobj == NULL) 
					PyErr_Print();
				else {
					// remember the this pointer
					PyObject *th = PyLong_FromVoidPtr(this); 
					int ret = PyObject_SetAttrString(pyobj,"_this",th);

					// now get number of inlets and outlets
					inlets = 1,outlets = 1;

					PyObject *res;
					res = PyObject_GetAttrString(pyobj,"_inlets"); 
					if(res) {
						if(PyCallable_Check(res)) {
							PyObject *fres = PyEval_CallObject(res,NULL);
							Py_DECREF(res);
							res = fres;
						}
						if(PyInt_Check(res)) 
							inlets = PyInt_AsLong(res);
						Py_DECREF(res);
					}
					else 
						PyErr_Clear();

					res = PyObject_GetAttrString(pyobj,"_outlets"); 
					if(res) {
						if(PyCallable_Check(res)) {
							PyObject *fres = PyEval_CallObject(res,NULL);
							Py_DECREF(res);
							res = fres;
						}
						if(PyInt_Check(res))
							outlets = PyInt_AsLong(res);
						Py_DECREF(res);
					}
					else
						PyErr_Clear();
				}
			}
			else 
				post("%s - Type of \"%s\" is unhandled!",thisName(),GetString(sobj));
		}
	}

	PY_UNLOCK
	
	AddInAnything(1+inlets);  
	AddOutAnything(outlets);  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDMETHOD_(0,"reload",m_reload);

#ifdef FLEXT_THREADS
	FLEXT_ADDMETHOD_(0,"detach",m_detach);
	FLEXT_ADDMETHOD_(0,"stop",m_stop);
#endif

	if(!pyobj)
		InitProblem();
}

pyext::~pyext()
{
	PY_LOCK
	
	Py_XDECREF(pyobj);

/*
	// Don't unregister

	if(pyref == 1) {
		Py_DECREF(class_obj);
		class_obj = NULL;
		Py_DECREF(class_dict);
		class_dict = NULL;
	}
*/
	PY_UNLOCK
}

V pyext::m_reload(I argc,t_atom *argv)
{
	PY_LOCK
	if(argc > 2) SetArgs(argc,argv);

	ReloadModule();
	PY_UNLOCK
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
	post("\treload {args...}: reload python script");
#ifdef FLEXT_THREADS
	post("\tdetach 0/1: detach threads");
	post("\tstop {wait time (ms)}: stop threads");
#endif
	post("");
}

PyObject *pyext::call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv) 
{
	PyObject *ret = NULL;

	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
	}
	else {
		PyObject *pargs = MakePyArgs(s,argc,argv,inlet?inlet:-1,true);
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

	return ret;
}

V pyext::work_wrapper(V *data)
{
	++thrcount;
	work_data *w = (work_data *)data;
	work(w->n,w->Header(),w->Count(),w->Atoms());
	delete w;
	--thrcount;
}

BL pyext::callwork(I n,const t_symbol *s,I argc,t_atom *argv)
{
	if(detach) {
		if(shouldexit) {
			post("%s - New threads can't be launched now!",thisName());
			return false;
		}
		else {
			FLEXT_CALLMETHOD_X(work_wrapper,new work_data(n,s,argc,argv));
			return true;
		}
	}
	else 
		return work(n,s,argc,argv);
}

BL pyext::work(I n,const t_symbol *s,I argc,t_atom *argv)
{
	BL retv = false;

	PY_LOCK

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
		if(!PyObject_Not(ret)) post("%s - returned value is ignored",thisName());
		Py_DECREF(ret);
		retv = true;
	}

	PY_UNLOCK

	return retv;
}



