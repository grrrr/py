/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"
#include <flinternal.h>


FLEXT_LIB_V("pyext",pyext)

V pyext::setup(t_class *)
{
	px_head = px_tail = NULL;

	px_class = class_new(gensym("pyext proxy"),NULL,NULL,sizeof(py_proxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	::add_anything(px_class,py_proxy::px_method); // for other inlets
}

pyext *pyext::GetThis(PyObject *self)
{
	PyObject *th = PyObject_GetAttrString(self,"_this");
	pyext *ret = th?(pyext *)PyLong_AsVoidPtr(th):NULL;
	PyErr_Clear();
	Py_XDECREF(th);
	return ret;
}


I pyext::pyextref = 0;
PyObject *pyext::class_obj = NULL;
PyObject *pyext::class_dict = NULL;

pyext::pyext(I argc,const t_atom *argv):
	pyobj(NULL),pythr(NULL),
	inlets(0),outlets(0),
	methname(NULL)
{ 
	PY_LOCK

	if(!pyextref++) {
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

#if PY_VERSION_HEX >= 0x02020000
		// not absolutely necessary, existent in python 2.2 upwards
		// make pyext functions available in class scope
		PyDict_Merge(class_dict,module_dict,0);
#endif

		PyDict_SetItemString(class_dict,"__doc__",PyString_FromString(pyext_doc));
 	}
	else {
		Py_INCREF(class_obj);
		Py_INCREF(class_dict);
	}

	// init script module
	if(argc >= 1) {
		C dir[1024];
#ifdef PD
		// add dir of current patch to path
		strcpy(dir,GetString(canvas_getdir(thisCanvas())));
		AddToPath(dir);
		// add current dir to path
		strcpy(dir,GetString(canvas_getcurrentdir()));
		AddToPath(dir);
#else
		#pragma message("Adding current dir to path is not implemented")
#endif

		GetModulePath(GetString(argv[0]),dir,sizeof(dir));
		// add to path
		AddToPath(dir);

		if(!IsString(argv[0])) 
			post("%s - script name argument is invalid",thisName());
		else {
			SetArgs(0,NULL);
			ImportModule(GetString(argv[0]));
		}
	}

	Register("_pyext");

//	t_symbol *sobj = NULL;
	if(argc >= 2) {
		// object name
		if(!IsString(argv[1])) 
			post("%s - object name argument is invalid",thisName());
		else {
			methname = GetSymbol(argv[1]);
		}

		args(argc-2,argv+2);
	}

	if(methname) {
		SetClssMeth();

		// now get number of inlets and outlets
		inlets = 1,outlets = 1;

		if(pyobj) {
			PyObject *res;
			res = PyObject_GetAttrString(pyobj,"_inlets"); // get ref
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

			res = PyObject_GetAttrString(pyobj,"_outlets"); // get ref
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

	PY_UNLOCK
	
	AddInAnything(1+inlets);  
	AddOutAnything(outlets);  

	FLEXT_ADDMETHOD_(0,"reload.",m_reload);
	FLEXT_ADDMETHOD_(0,"reload",m_reload_);
	FLEXT_ADDMETHOD_(0,"doc",m_doc);
	FLEXT_ADDMETHOD_(0,"doc+",m_doc_);

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

	ClearBinding();
	Unregister("_pyext");
	
	Py_XDECREF(pyobj);

	Py_XDECREF(class_obj);
	Py_XDECREF(class_dict);
/*
	// Don't unregister

	if(!--pyextref) {
		class_obj = NULL;
		class_dict = NULL;
	}
*/
	PY_UNLOCK
}

BL pyext::SetClssMeth() //I argc,t_atom *argv)
{
	// pyobj should already have been decref'd / cleared before getting here!!
	
	if(module && methname) {
		PyObject *pref = PyObject_GetAttrString(module,const_cast<C *>(GetString(methname)));  
		if(!pref) 
			PyErr_Print();
		else if(PyClass_Check(pref)) {
			// make instance, but don't call __init__ 
			pyobj = PyInstance_NewRaw(pref,NULL);

			Py_DECREF(pref);
			if(pyobj == NULL) 
				PyErr_Print();
			else {
				// remember the this pointer
				PyObject *th = PyLong_FromVoidPtr(this); 
				int ret = PyObject_SetAttrString(pyobj,"_this",th); // ref is taken

				// call init now, after _this has been set, which is
				// important for eventual callbacks from __init__ to c
				PyObject *pargs = MakePyArgs(NULL,args,-1,true);
				if (pargs == NULL) PyErr_Print();

				PyObject *init;
				init = PyObject_GetAttrString(pyobj,"__init__"); // get ref
				if(init && PyCallable_Check(init)) {
					PyObject *res = PyEval_CallObject(init,pargs);
					if(!res)
						PyErr_Print();
					else
						Py_DECREF(res);
				}
				
				Py_XDECREF(pargs);
			}
		}
		else 
			post("%s - Type of \"%s\" is unhandled!",thisName(),GetString(methname));
		return true;
	}
	else
		return false;
}

V pyext::Reload()
{
	ClearBinding();
	Py_XDECREF(pyobj);
	// by here, the Python class destructor should have been called!

	SetArgs(0,NULL);
	ReloadModule();
	
	SetClssMeth();
}


V pyext::m_reload()
{
	PY_LOCK

	Unregister("_pyext"); // self

	Reload();

	Reregister("_pyext"); // the others
	Register("_pyext"); // self

	PY_UNLOCK
}

V pyext::m_reload_(I argc,const t_atom *argv)
{
	args(argc,argv);
	m_reload();
}

V pyext::m_doc_()
{
	if(pyobj) {
		PY_LOCK

		PyObject *docf = PyObject_GetAttrString(pyobj,"__doc__"); // borrowed!!!
		if(docf && PyString_Check(docf)) {
			post("");
			post(PyString_AsString(docf));
		}

		PY_UNLOCK
	}
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
	post("");
	post("pyext %s - python script object, (C)2002 Thomas Grill",PY__VERSION);
#ifdef _DEBUG
	post("compiled on " __DATE__ " " __TIME__);
#endif

	post("Arguments: %s [script name] [class name] {args...}",thisName());

	post("Inlet 1: messages to control the pyext object");
	post("      2...: python inlets");
	post("Outlets: python outlets");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\treload {args...}: reload python script");
	post("\treload. : reload with former arguments");
	post("\tdoc: display module doc string");
	post("\tdoc+: display class doc string");
#ifdef FLEXT_THREADS
	post("\tdetach 0/1: detach threads");
	post("\tstop {wait time (ms)}: stop threads");
#endif
	post("");
}

PyObject *pyext::call(const C *meth,I inlet,const t_symbol *s,I argc,const t_atom *argv) 
{
	PyObject *ret = NULL;

	PyObject *pmeth  = PyObject_GetAttrString(pyobj,const_cast<char *>(meth)); /* fetch bound method */
	if(pmeth == NULL) {
		PyErr_Clear(); // no method found
	}
	else {
		PyObject *pargs = MakePyArgs(s,AtomList(argc,argv),inlet?inlet:-1,true);
		if(!pargs)
			PyErr_Print();
		else {
			ret = PyEval_CallObject(pmeth, pargs); 
			if (ret == NULL) // function not found resp. arguments not matching
#if 1 //def _DEBUG
				PyErr_Print();
#else
				PyErr_Clear();  
#endif
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
#ifdef _DEBUG
	if(!data) 
		post("%s - no data!",thisName());
	else
#endif
	{
		work_data *w = (work_data *)data;
		work(w->n,w->Header(),w->Count(),w->Atoms());
//		delete w;
	}
	--thrcount;
}

BL pyext::callwork(I n,const t_symbol *s,I argc,const t_atom *argv)
{
	if(detach) {
		if(shouldexit) {
			post("%s - Stopping.... new threads can't be launched now!",thisName());
			return true;
		}
		else {
			BL ret = FLEXT_CALLMETHOD_X(work_wrapper,new work_data(n,s,argc,argv));
			if(!ret) post("%s - Failed to launch thread!",thisName());
			return true;
		}
	}
	else 
		return work(n,s,argc,argv);
}

BL pyext::work(I n,const t_symbol *s,I argc,const t_atom *argv)
{
	BL retv = false;

	PY_LOCK

	PyObject *ret = NULL;
	char *str = new char[strlen(GetString(s))+10];

	{
		// try tag/inlet
		sprintf(str,"%s_%i",GetString(s),n);
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
		sprintf(str,"%s_",GetString(s));
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



