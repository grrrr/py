/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"
#include <flinternal.h>

FLEXT_LIB_V("pyext pyx",pyext)

V pyext::Setup(t_classid c)
{
	FLEXT_CADDMETHOD_(c,0,"reload.",m_reload);
	FLEXT_CADDMETHOD_(c,0,"reload",m_reload_);
	FLEXT_CADDMETHOD_(c,0,"dir",m_dir);
	FLEXT_CADDMETHOD_(c,0,"dir+",m_dir_);
	FLEXT_CADDMETHOD_(c,0,"doc",m_doc);
	FLEXT_CADDMETHOD_(c,0,"doc+",m_doc_);

  	FLEXT_CADDATTR_VAR(c,"args",args,ms_args);
	FLEXT_CADDATTR_GET(c,"dir+",mg_dir_);

#ifdef FLEXT_THREADS
	FLEXT_CADDATTR_VAR1(c,"detach",detach);
	FLEXT_CADDMETHOD_(c,0,"stop",m_stop);
#endif

	FLEXT_CADDMETHOD_(c,0,"get",m_get);
	FLEXT_CADDMETHOD_(c,0,"set",m_set);

  	FLEXT_CADDATTR_VAR1(c,"respond",respond);


	// ----------------------------------------------------

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
	Py_DECREF(className);

	// add methods to class 
	for (def = meth_tbl; def->ml_name != NULL; def++) {
		PyObject *func = PyCFunction_New(def, NULL);
		PyObject *method = PyMethod_New(func, NULL, class_obj); // increases class_obj ref count by 1
		PyDict_SetItemString(class_dict, def->ml_name, method);
		Py_DECREF(func);
		Py_DECREF(method);
	}

#if PY_VERSION_HEX >= 0x02020000
	// not absolutely necessary, existent in python 2.2 upwards
	// make pyext functions available in class scope
	PyDict_Merge(class_dict,module_dict,0);
#endif
	// after merge so that it's not in class_dict as well...
	PyDict_SetItemString(module_dict, PYEXT_CLASS,class_obj); // increases class_obj ref count by 1

	PyDict_SetItemString(class_dict,"__doc__",PyString_FromString(pyext_doc));
}

pyext *pyext::GetThis(PyObject *self)
{
	PyObject *th = PyObject_GetAttrString(self,"_this");
	pyext *ret = th?(pyext *)PyLong_AsVoidPtr(th):NULL;
	PyErr_Clear();
	Py_XDECREF(th);
	return ret;
}


#if FLEXT_SYS == FLEXT_SYS_MAX
static short patcher_myvol(t_patcher *x)
{
    t_box *w;
    if (x->p_vol)
        return x->p_vol;
    else if (w = (t_box *)x->p_vnewobj)
        return patcher_myvol(w->b_patcher);
    else
        return 0;
}
#endif


PyObject *pyext::class_obj = NULL;
PyObject *pyext::class_dict = NULL;

pyext::pyext(I argc,const t_atom *argv):
	pyobj(NULL),pythr(NULL),
	inlets(-1),outlets(-1),
	methname(NULL)
{ 
    int apre = 0;

    if(argc >= apre+2 && CanbeInt(argv[apre]) && CanbeInt(argv[apre+1])) {
        inlets = GetAInt(argv[apre]);
        outlets = GetAInt(argv[apre+1]);
        apre += 2;
    }

    PY_LOCK

	// init script module
	if(argc > apre) {
		char dir[1024];

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
        const t_atom &scr = argv[apre];

		if(!IsString(scr)) 
			post("%s - script name argument is invalid",thisName());
		else {
		    GetModulePath(GetString(scr),dir,sizeof(dir));
		    // add to path
		    AddToPath(dir);

			SetArgs(0,NULL);

			ImportModule(GetString(scr));
		}

        ++apre;
	}

 	Register("_pyext");

//	t_symbol *sobj = NULL;
	if(argc > apre) {
		// object name
		if(!IsString(argv[apre])) 
			post("%s - object name argument is invalid",thisName());
		else {
			methname = GetSymbol(argv[apre]);
		}

        ++apre;
	}

	if(argc > apre) args(argc-apre,argv+apre);

	if(methname) {
		MakeInstance();

        if(pyobj) {
            if(inlets >= 0) {
                // set number of inlets
			    PyObject *res = PyInt_FromLong(inlets);
                int ret = PyObject_SetAttrString(pyobj,"_inlets",res);
                FLEXT_ASSERT(!ret);
            }
            if(outlets >= 0) {
                // set number of outlets
			    PyObject *res = PyInt_FromLong(outlets);
			    int ret = PyObject_SetAttrString(pyobj,"_outlets",res);
                FLEXT_ASSERT(!ret);
            }

            DoInit(); // call __init__ constructor
            // __init__ can override the number of inlets and outlets

            if(inlets < 0) {
		        // get number of inlets
		        inlets = 1;
			    PyObject *res = PyObject_GetAttrString(pyobj,"_inlets"); // get ref
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
            }
            if(outlets < 0) {
                // get number of outlets
                outlets = 1;
			    PyObject *res = PyObject_GetAttrString(pyobj,"_outlets"); // get ref
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
	}
    else 
        inlets = outlets = 0;

	PY_UNLOCK
	
    FLEXT_ASSERT(inlets >= 0 && outlets >= 0);

	AddInAnything(1+inlets);  
	AddOutAnything(outlets);  

	if(!pyobj)
		InitProblem();
}

pyext::~pyext()
{
	PY_LOCK

	ClearBinding();
	Unregister("_pyext");
	UnimportModule();

	Py_XDECREF(pyobj);  // opposite of SetClssMeth

	PY_UNLOCK
}

BL pyext::DoInit()
{
	// remember the this pointer
	PyObject *th = PyLong_FromVoidPtr(this); 
	int ret = PyObject_SetAttrString(pyobj,"_this",th); // ref is taken

	// call init now, after _this has been set, which is
	// important for eventual callbacks from __init__ to c
	PyObject *pargs = MakePyArgs(NULL,args.Count(),args.Atoms(),-1,true);
	if(!pargs) PyErr_Print();

	PyObject *init = PyObject_GetAttrString(pyobj,"__init__"); // get ref
    if(init) {
        if(PyCallable_Check(init)) {
			PyObject *res = PyEval_CallObject(init,pargs);
			if(!res)
				PyErr_Print();
			else
				Py_DECREF(res);
        }
        Py_DECREF(init);
	}
    
	Py_XDECREF(pargs);
    return true;
}

BL pyext::MakeInstance()
{
	// pyobj should already have been decref'd / cleared before getting here!!
	
	if(module && methname) {
		PyObject *pref = PyObject_GetAttrString(module,const_cast<C *>(GetString(methname)));  
		if(!pref) 
			PyErr_Print();
        else {
            if(PyClass_Check(pref)) {
			    // make instance, but don't call __init__ 
			    pyobj = PyInstance_NewRaw(pref,NULL);

			    if(!pyobj) PyErr_Print();
            }
            else
			    post("%s - Type of \"%s\" is unhandled!",thisName(),GetString(methname));

		    Py_DECREF(pref);
		}
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
	
	MakeInstance();
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

void pyext::m_get(const t_symbol *s)
{
    PY_LOCK

	PyObject *pvar  = PyObject_GetAttrString(pyobj,const_cast<char *>(GetString(s))); /* fetch bound method */
	if(!pvar) {
		PyErr_Clear(); // no method found
        post("%s - get: Python variable %s not found",thisName(),GetString(s));
	}
	else {
        AtomList *lst = GetPyArgs(pvar);
        if(lst) {
            // dump value to attribute outlet
            AtomAnything out("get",lst->Count()+1);
            SetSymbol(out[0],s);
            out.Set(lst->Count(),lst->Atoms(),1);
            delete lst;

            ToOutAnything(GetOutAttr(),out);
        }
        else
            post("%s - get: List could not be created",thisName());
        Py_DECREF(pvar);
    }

    PY_UNLOCK
}

void pyext::m_set(int argc,const t_atom *argv)
{
    PY_LOCK

    if(argc < 2 || !IsString(argv[0]))
        post("%s - Syntax: set varname arguments...",thisName());
    else if(*GetString(argv[0]) == '_')
        post("%s - set: variables with leading _ are reserved and can't be set",thisName());
    else {
        char *ch = const_cast<char *>(GetString(argv[0]));
        if(!PyObject_HasAttrString(pyobj,ch)) {
		    PyErr_Clear(); // no method found
            post("%s - set: Python variable %s not found",thisName(),ch);
	    }
	    else {
            PyObject *pval = MakePyArgs(NULL,argc-1,argv+1,-1,false);

            if(!pval)
			    PyErr_Print();
		    else {
                if(PySequence_Size(pval) == 1) {
                    // reduce lists of one element to element itself

                    PyObject *val1 = PySequence_GetItem(pval,0);
                    Py_DECREF(pval);
                    pval = val1;
                }

                PyObject_SetAttrString(pyobj,ch,pval);
                Py_DECREF(pval);
            }
        }
    }

    PY_UNLOCK
}


BL pyext::m_method_(I n,const t_symbol *s,I argc,const t_atom *argv)
{
    BL ret = false;
	if(pyobj && n >= 1)
		ret = callwork(n,s,argc,argv);
    else
		post("%s - no method for type '%s' into inlet %i",thisName(),GetString(s),n);
    return ret;
}


V pyext::m_help()
{
	post("");
	post("%s %s - python class object, (C)2002-2004 Thomas Grill",thisName(),PY__VERSION);
#ifdef FLEXT_DEBUG
	post("DEBUG VERSION, compiled on " __DATE__ " " __TIME__);
#endif

    post("Arguments: %s {inlets outlets} [script name] [class name] {args...}",thisName());

	post("Inlet 1: messages to control the pyext object");
	post("      2...: python inlets");
	post("Outlets: python outlets");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\treload {args...}: reload python script");
	post("\treload. : reload with former arguments");
	post("\tdoc: display module doc string");
	post("\tdoc+: display class doc string");
	post("\tdir: dump module dictionary");
	post("\tdir+: dump class dictionary");
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
		PyObject *pargs = MakePyArgs(s,argc,argv,inlet?inlet:-1,true);
		if(!pargs)
			PyErr_Print();
		else {
			ret = PyEval_CallObject(pmeth, pargs); 
			if (ret == NULL) // function not found resp. arguments not matching
				PyErr_Print();

			Py_DECREF(pargs);
		}
		Py_DECREF(pmeth);
	}

	return ret;
}

V pyext::work_wrapper(V *data)
{
	++thrcount;
#ifdef FLEXT_DEBUG
	if(!data) 
		post("%s - no data!",thisName());
	else
#endif
	{
#ifdef FLEXT_THREADS
        // --- make new Python thread ---
        // get the global lock
        PyEval_AcquireLock();
        // create a thread state object for this thread
        PyThreadState *newthr = PyThreadState_New(pystate);
        // free the lock
        PyEval_ReleaseLock();
        // -----------------------------

        // store new thread state
        pythrmap[GetThreadId()] = newthr;
#endif
        {
            // call worker
		    work_data *w = (work_data *)data;
		    work(w->n,w->Header(),w->Count(),w->Atoms());
		    delete w;
        }

#ifdef FLEXT_THREADS
        // delete mapped thread state
        pythrmap.erase(GetThreadId());

        // --- delete Python thread ---
        // grab the lock
        PyEval_AcquireLock();
        // swap my thread state out of the interpreter
        PyThreadState_Swap(NULL);
        // clear out any cruft from thread state object
        PyThreadState_Clear(newthr);
        // delete my thread state object
        PyThreadState_Delete(newthr);
        // release the lock
        PyEval_ReleaseLock();
        // -----------------------------
#endif
	}
	--thrcount;
}

BL pyext::callwork(I n,const t_symbol *s,I argc,const t_atom *argv)
{
    BL ret = true,ok = false;
    if(detach) {
		if(shouldexit)
			post("%s - Stopping.... new threads can't be launched now!",thisName());
		else {
			ret = FLEXT_CALLMETHOD_X(work_wrapper,new work_data(n,s,argc,argv));
			if(!ret) post("%s - Failed to launch thread!",thisName());
		}
	}
    else
		ret = ok = work(n,s,argc,argv);
    Respond(ok);
    return ret;
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
