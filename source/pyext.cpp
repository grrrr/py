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

	static PyObject *py_new(PyObject *self,PyObject *args);
	static PyObject *py__init__(PyObject *self,PyObject *args);
	static PyObject *py_outlet(PyObject *self,PyObject *args);

	I Inlets() const { return inlets; }
	I Outlets() const { return outlets; }

protected:
	BL m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	virtual BL work(I n,const t_symbol *s,I argc,t_atom *argv); 

//	virtual V m_bang() { work(sym_bang,0,NULL); }
	virtual V m_reload(I argc,t_atom *argv);
	virtual V m_set(I argc,t_atom *argv);

	virtual V m_help();

	PyObject *pyobj;
	I inlets,outlets;

private:
	enum retval { nothing,atom,tuple,list };

	static V setup(t_class *);
	
	PyObject *call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv);
	PyObject *call(const C *meth);

//	FLEXT_CALLBACK(m_bang)

	FLEXT_CALLBACK_V(m_reload)
	FLEXT_CALLBACK_V(m_set)
};

FLEXT_LIB_V("pyext",pyext)


PyObject* pyext::py__init__(PyObject *sl, PyObject *args)
{
//    post("pyext.__init__ called");

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *pyext::py_outlet(PyObject *sl,PyObject *args) 
{
//    post("pyext.outlet called");

	I rargc;
	PyObject *self;
	t_atom *rargv = GetPyArgs(rargc,args,&self);

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
    return Py_None;
}


static PyMethodDef pyext_meths[] = 
{
//    {"new", pyext::py_new, METH_VARARGS, "pyext new"},
//    {"__init__", pyext::py__init__, METH_VARARGS, "pyext init"},
    {"_outlet", pyext::py_outlet, METH_VARARGS,"pyext outlet"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyMethodDef pyext_mod_meths[] = {{NULL, NULL, 0, NULL}};

static I ref = 0;

pyext::pyext(I argc,t_atom *argv):
	pyobj(NULL)
{ 
	if(!(ref++)) {
		PyObject *module = Py_InitModule("pyext", pyext_mod_meths);

		PyObject *moduleDict = PyModule_GetDict(module);
		PyObject *classDict = PyDict_New();
		PyObject *className = PyString_FromString("base");
		PyObject *fooClass = PyClass_New(NULL, classDict, className);
		PyDict_SetItemString(moduleDict, "base", fooClass);
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
#ifdef PD
		// uarghh... pd doesn't show it's path for extra modules

		C *name;
		I fd = open_via_path("",GetString(argv[0]),".py",dir,&name,sizeof(dir),0);
		if(fd > 0) close(fd);
		else name = NULL;

		// if dir is current working directory... name points to dir
		if(dir == name) strcpy(dir,".");
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
	AddOutAnything(1+outlets);  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDMETHOD_(0,"reload",m_reload);
	FLEXT_ADDMETHOD_(0,"set",m_set);

//	if(!pyobj) InitProblem();
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

PyObject *pyext::call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv) 
{
	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
		return NULL;
	}
	else {
		PyObject *pres;
		PyObject *pargs = MakePyArgs(s,argc,argv,inlet?inlet:-1);
		if(!pargs)
			PyErr_Print();
		else {
			pres = PyEval_CallObject(pmeth, pargs);           /* call method(x,y) */
			if (pres == NULL)
				PyErr_Print();
			else {
//				Py_DECREF(pres);
			}

			Py_DECREF(pargs);
		}
		Py_DECREF(pmeth);

		return pres;
	}
}

PyObject *pyext::call(const C *meth) 
{
	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
		return NULL;
	}
	else {
//		PyObject *sf = PyTuple_New(0);
//		PyTuple_SetItem(sf,0,pyobj); 

		PyObject *pres = PyEval_CallObject(pmeth, NULL);           /* call method(x,y) */
		if (pres == NULL)
			PyErr_Print();
		else {
//			Py_DECREF(pres);
		}
		Py_DECREF(pmeth);
//		Py_DECREF(sf);

		return pres;
	}
}

BL pyext::work(I n,const t_symbol *s,I argc,t_atom *argv)
{
	PyObject *ret = NULL;
	char *str = new char[strlen(GetString(s))+10];

	{
		sprintf(str,"_%s_%i",GetString(s),n);
		ret = call(str,0,NULL,argc,argv);
	}
	if(!ret) {
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
		sprintf(str,"_%s_",GetString(s));
		ret = call(str,n,NULL,argc,argv);
	}
	if(!ret) {
		strcpy(str,"_anything_");
		if(s == sym_bang && !argc) {
			t_atom argv;
			SetString(argv,"");
			ret = call(str,n,s,1,&argv);
		}
		else
			ret = call(str,n,s,argc,argv);
	}
	if(!ret) post("%s - no matching method found for '%s' into inlet %i",thisName(),GetString(s),n);

	if(str) delete[] str;

	if(ret) {
		if(!PyObject_Not(ret)) post("%s - returned value is ignored",thisName());
		Py_DECREF(ret);
		return true;
	}
	return false;
}



