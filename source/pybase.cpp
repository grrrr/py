/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pybase.h"
#include <map>

static PyMethodDef StdOut_Methods[] =
{
	{ "write", pybase::StdOut_Write, 1 },
	{ NULL,    NULL,           }  
};

static PyObject *gcollect = NULL;

#ifdef FLEXT_THREADS

typedef std::map<flext::thrid_t,PyThreadState *> PyThrMap;

static PyInterpreterState *pymain = NULL;
static PyThrMap pythrmap;
PyThreadState *pybase::pythrsys = NULL;

int pybase::lockcount = 0;

PyThreadState *pybase::FindThreadState()
{
    flext::thrid_t id = flext::GetThreadId();
	PyThrMap::iterator it = pythrmap.find(id);
    if(it == pythrmap.end()) {
        // Make new thread state
        PyThreadState *st = PyThreadState_New(pymain);
        pythrmap[id] = st;
        return st;
    }
    else 
        return it->second;
}

void pybase::FreeThreadState()
{
    flext::thrid_t id = flext::GetThreadId();
	PyThrMap::iterator it = pythrmap.find(id);
    if(it != pythrmap.end()) {
        // clear out any cruft from thread state object
        PyThreadState_Clear(it->second);
        // delete my thread state object
        PyThreadState_Delete(it->second);
        // delete from map
        pythrmap.erase(it);
    }
}

PyFifo pybase::qufifo;
flext::ThrCond pybase::qucond;
#endif


PyObject *pybase::module_obj = NULL;
PyObject *pybase::module_dict = NULL;

PyObject *pybase::builtins_obj = NULL;
PyObject *pybase::builtins_dict = NULL;

const t_symbol *pybase::sym_fint = NULL;

// -----------------------------------------------------------------------------------------------------------


void initsymbol();
void initsamplebuffer();

void pybase::lib_setup()
{
	post("");
	post("------------------------------------------------");
	post("py/pyext %s - python script objects",PY__VERSION);
	post("(C)2002-2005 Thomas Grill - http://grrrr.org/ext");
    post("");
    post("using Python %s",Py_GetVersion());

#ifdef FLEXT_DEBUG
    post("");
	post("DEBUG version compiled on %s %s",__DATE__,__TIME__);
#endif

	// -------------------------------------------------------------

	Py_Initialize();

#ifdef FLEXT_DEBUG
//	Py_VerboseFlag = 1;
#endif

#ifdef FLEXT_THREADS
    // enable thread support and acquire the global thread lock
	PyEval_InitThreads();

    // get thread state
    pythrsys = PyThreadState_Get();
    // get main interpreter state
	pymain = pythrsys->interp;

    // add thread state of main thread to map
    pythrmap[GetThreadId()] = pythrsys;
#endif

    // sys.argv must be set to empty tuple
    char *nothing = "";
	PySys_SetArgv(0,&nothing);

    // register/initialize pyext module only once!
	module_obj = Py_InitModule(PYEXT_MODULE, func_tbl);
	module_dict = PyModule_GetDict(module_obj); // borrowed reference

	PyModule_AddStringConstant(module_obj,"__doc__",(char *)py_doc);

	// redirect stdout
	PyObject* py_out;
    py_out = Py_InitModule("stdout", StdOut_Methods);
	PySys_SetObject("stdout", py_out);
    py_out = Py_InitModule("stderr", StdOut_Methods);
	PySys_SetObject("stderr", py_out);

    // get garbage collector function
    PyObject *gcobj = PyImport_ImportModule("gc");
    if(gcobj) {
        gcollect = PyObject_GetAttrString(gcobj,"collect");
        Py_DECREF(gcobj);
    }

    builtins_obj = PyImport_ImportModule("__builtin__");
	builtins_dict = PyModule_GetDict(builtins_obj); // borrowed reference

    // add symbol type
    initsymbol();
    PyModule_AddObject(module_obj,"Symbol",(PyObject *)&pySymbol_Type);

    // pre-defined symbols
    PyModule_AddObject(module_obj,"_s_",(PyObject *)pySymbol__);
    PyModule_AddObject(module_obj,"_s_bang",(PyObject *)pySymbol_bang);
    PyModule_AddObject(module_obj,"_s_list",(PyObject *)pySymbol_list);
    PyModule_AddObject(module_obj,"_s_symbol",(PyObject *)pySymbol_symbol);
    PyModule_AddObject(module_obj,"_s_float",(PyObject *)pySymbol_float);
    PyModule_AddObject(module_obj,"_s_int",(PyObject *)pySymbol_int);

    // add samplebuffer type
    initsamplebuffer();
    PyModule_AddObject(module_obj,"Buffer",(PyObject *)&pySamplebuffer_Type);

#if FLEXT_SYS == FLEXT_SYS_PD
    sym_fint = sym_float;
#else
    sym_fint = sym_int;
#endif

	// -------------------------------------------------------------

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pymeth);
	FLEXT_SETUP(pyext);
	FLEXT_DSP_SETUP(pydsp);

#ifdef FLEXT_THREADS
    // release global lock
    PyEval_ReleaseLock();

    // launch thread worker
    LaunchThread(quworker,NULL);
#endif

	post("------------------------------------------------");
    post("");
}

FLEXT_LIB_SETUP(py,pybase::lib_setup)


// -----------------------------------------------------------------------------------------------------------


pybase::pybase()
    : module(NULL)
#ifdef FLEXT_THREADS
    , thrcount(0)
    , shouldexit(false),stoptick(0)
#endif
	, detach(0)
    , pymsg(false)
{
    PyThreadState *state = PyLockSys();
	Py_INCREF(module_obj);
    PyUnlock(state);
}

pybase::~pybase()
{
    PyThreadState *state = PyLockSys();
   	Py_XDECREF(module_obj);
    PyUnlock(state);
}

void pybase::Exit()
{
#ifdef FLEXT_THREADS
    erasethreads();

    shouldexit = true;
    qucond.Signal();

    if(thrcount) {
		// Wait for a certain time
		for(int i = 0; i < (PY_STOP_WAIT/PY_STOP_TICK) && thrcount; ++i) 
            Sleep(PY_STOP_TICK*0.001f);
        if(thrcount) {
		    // Wait forever
		    post("py/pyext - Waiting for thread termination!");
		    while(thrcount) Sleep(PY_STOP_TICK*0.001f);
		    post("py/pyext - Okay, all threads have terminated");
        }
	}
#endif
}

void pybase::GetDir(PyObject *obj,AtomList &lst)
{
    if(obj) {
        PyThreadState *state = PyLock();
    
        PyObject *pvar  = PyObject_Dir(obj);
	    if(!pvar)
		    PyErr_Print(); // no method found
	    else {
            const t_symbol *sym = GetPyArgs(lst,pvar);
            if(!sym)
                post("py/pyext - Argument list could not be created");
            else
                FLEXT_ASSERT(sym == sym_list);
            Py_DECREF(pvar);
        }

        PyUnlock(state);
    }
}

void pybase::m__dir(PyObject *obj)
{
    AtomList lst;
    GetDir(obj,lst);
    // dump dir to attribute outlet
    DumpOut(NULL,lst.Count(),lst.Atoms());
}

void pybase::m__doc(PyObject *obj)
{
    if(obj) {
        PyThreadState *state = PyLock();

		PyObject *docf = PyDict_GetItemString(obj,"__doc__"); // borrowed!!!
		if(docf && PyString_Check(docf)) {
			post("");
			const char *s = PyString_AS_STRING(docf);

			// FIX: Python doc strings can easily be larger than 1k characters
			// -> split into separate lines
			for(;;) {
				char buf[1024];
				char *nl = strchr((char *)s,'\n'); // the cast is for Borland C++
				if(!nl) {
					// no more newline found
					post(s);
					break;
				}
				else {
					// copy string before newline to temp buffer and post
					unsigned int l = nl-s;
					if(l >= sizeof(buf)) l = sizeof buf-1;
					strncpy(buf,s,l); // copy all but newline
					buf[l] = 0;
					post(buf);
					s = nl+1;  // set after newline
				}
			}
		}

        PyUnlock(state);
	}
}

void pybase::OpenEditor()
{
    // this should once open the editor....
}

void pybase::SetArgs()
{
	// script arguments
    int argc = args.Count();
    const t_atom *argv = args.Atoms();
	char **sargv = new char *[argc+1];
	for(int i = 0; i <= argc; ++i) {
		sargv[i] = new char[256];
		if(!i) 
			strcpy(sargv[i],"py/pyext");
		else
			GetAString(argv[i-1],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc+1,sargv);

	for(int j = 0; j <= argc; ++j) delete[] sargv[j];
	delete[] sargv;
}

bool pybase::ImportModule(const char *name)
{
    if(name) {
        if(modname == name) return true;
        modname = name;
    }
    else
        modname.clear();
    return ReloadModule();
}

void pybase::UnimportModule()
{
	if(!module) return;

	FLEXT_ASSERT(dict && module_obj && module_dict);

	Py_DECREF(module);

	// reference count to module is not 0 here, altough probably the last instance was unloaded
	// Python retains one reference to the module all the time 
	// we don't care

	module = NULL;
	dict = NULL;
}

bool pybase::ReloadModule()
{
    bool ok = false;

    SetArgs();
    PyObject *newmod;
    
    if(modname.length())
        newmod = module
            ?PyImport_ReloadModule(module)
            :PyImport_ImportModule((char *)modname.c_str());
    else {
        // if no module name given, take py module
        newmod = module_obj; 
        Py_INCREF(newmod);
    }

	if(!newmod) {
		// unload faulty module
        if(module) UnimportModule();
	}
	else {
		Py_XDECREF(module);
		module = newmod;
		dict = PyModule_GetDict(module); // borrowed
        ok = true;
	}
    return ok;
}

void pybase::GetModulePath(const char *mod,char *dir,int len)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	// uarghh... pd doesn't show its path for extra modules

	char *name;
	int fd = open_via_path("",mod,".py",dir,&name,len,0);
	if(fd > 0) close(fd);
	else name = NULL;

	// if dir is current working directory... name points to dir
	if(dir == name) strcpy(dir,".");
#elif FLEXT_SYS == FLEXT_SYS_MAX
	// how do i get the path in Max/MSP?
    short path;
    long type;
    char smod[1024];
    strcat(strcpy(smod,mod),".py");
    if(!locatefile_extended(smod,&path,&type,&type,-1)) {
#if FLEXT_OS == FLEXT_OS_WIN
        path_topathname(path,NULL,dir);
#else
        // convert pathname to unix style
        path_topathname(path,NULL,smod);
        char *colon = strchr(smod,':');
        if(colon) {
            *colon = 0;
            strcpy(dir,"/Volumes/");
            strcat(dir,smod);
            strcat(dir,colon+1);
        }
        else
            strcpy(dir,smod);
#endif
    }
    else 
        // not found
        *dir = 0;
#else
	*dir = 0;
#endif
}

void pybase::AddToPath(const char *dir)
{
	if(dir && *dir) {
		PyObject *pobj = PySys_GetObject("path");
		if(pobj && PyList_Check(pobj)) {
    		PyObject *ps = PyString_FromString(dir);
            if(!PySequence_Contains(pobj,ps))
				PyList_Append(pobj,ps); // makes new reference
            Py_DECREF(ps);
		}
		PySys_SetObject("path",pobj); // steals reference to pobj
	}
}

void pybase::AddCurrentPath(t_canvas *cnv)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	// add dir of current patch to path
	AddToPath(GetString(canvas_getdir(cnv)));
	// add current dir to path
	AddToPath(GetString(canvas_getcurrentdir()));
#elif FLEXT_SYS == FLEXT_SYS_MAX 
	char dir[1024];
	short path = patcher_myvol(cnv);
	path_topathname(path,NULL,dir); 
	AddToPath(dir);       
#else 
	#pragma message("Adding current dir to path is not implemented")
#endif
}

bool pybase::OutObject(flext_base *ext,int o,PyObject *obj)
{
    flext::AtomListStatic<16> lst;
    const t_symbol *sym = pymsg?GetPyAtom(lst,obj):GetPyArgs(lst,obj);
    if(sym) {
        // call to outlet _outside_ the Mutex lock!
        // otherwise (if not detached) deadlock will occur
        ext->ToOutAnything(o,sym,lst.Count(),lst.Atoms());
        return true;
    }
    else
        return false;
}

static const t_symbol *sym_response = flext::MakeSymbol("response");

void pybase::Respond(bool b) 
{ 
    if(respond) { 
        t_atom a; 
        SetBool(a,b); 
        DumpOut(sym_response,1,&a); 
    } 
}

void pybase::Reload()
{
	PyThreadState *state = PyLockSys();

	PyObject *reg = GetRegistry(REGNAME);

    if(reg) {
        PyObject *key;
        int pos = 0;
        while(PyDict_Next(reg,&pos,&key,NULL)) {
            pybase *th = (pybase *)PyLong_AsLong(key);
            FLEXT_ASSERT(th);
            th->Unload();
        }

        UnloadModule();
    }

	bool ok = ReloadModule();

    if(ok) {
        LoadModule();

        if(reg) {
            SetRegistry(REGNAME,reg);

            PyObject *key;
            int pos = 0;
            while(PyDict_Next(reg,&pos,&key,NULL)) {
                pybase *th = (pybase *)PyLong_AsLong(key);
                FLEXT_ASSERT(th);
                th->Load();
            }
        }
        else
            Load();
    }

    Report();
	PyUnlock(state);
}

static PyObject *output = NULL;

// post to the console
PyObject* pybase::StdOut_Write(PyObject* self, PyObject* args)
{
    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);

	for(int i = 0; i < sz; ++i) {
		PyObject *val = PyTuple_GET_ITEM(args,i); // borrowed reference
		PyObject *str = PyObject_Str(val); // new reference
		char *cstr = PyString_AS_STRING(str);
		char *lf = strchr(cstr,'\n');

		// line feed in string
		if(!lf) {
			// no -> just append
            if(output)
				PyString_ConcatAndDel(&output,str); // str is decrefd
			else
				output = str; // take str reference
		}
		else {
			// yes -> append up to line feed, reset output buffer to string remainder
			PyObject *part = PyString_FromStringAndSize(cstr,lf-cstr); // new reference
            if(output)
				PyString_ConcatAndDel(&output,part); // str is decrefd	
			else
				output = part; // take str reference

            // output concatenated string
			post(PyString_AS_STRING(output));

			Py_DECREF(output);
			output = PyString_FromString(lf+1);  // new reference
		}
	}

    Py_INCREF(Py_None);
    return Py_None;
}


class work_data
{
public:
    work_data(PyObject *f,PyObject *a): fun(f),args(a) {}
    ~work_data() { Py_DECREF(fun); Py_DECREF(args); }

    PyObject *fun,*args;
};

bool pybase::gencall(PyObject *pmeth,PyObject *pargs)
{
	bool ret = false;

    // Now call method
    switch(detach) {
        case 0:
            ret = docall(pmeth,pargs);
        	Py_DECREF(pargs);
        	Py_DECREF(pmeth);
            break;
#ifdef FLEXT_THREADS
        case 1:
            // put call into queue
            ret = qucall(pmeth,pargs);
            break;
        case 2:
            // each call a new thread
            if(!shouldexit) {
                thr_params *p = new thr_params;
                p->cl = (flext_base *)this;
                p->var->_ext = new work_data(pmeth,pargs);
			    ret = LaunchThread(thrworker,p);
			    if(!ret) post("py/pyext - Failed to launch thread!");
		    }
            break;
#endif
        default:
            post("py/pyext - Unknown detach mode");
    }
    return ret;
}

void pybase::exchandle()
{
#if 0
    // want to use that, but exception keeps a reference to the object
    PyErr_Print();
#else
    // must use that instead... clear the exception
    PyObject *type,*value,*traceback;
    PyErr_Fetch(&type,&value,&traceback);
    PyErr_NormalizeException(&type,&value,&traceback);
    PyErr_Display(type,value,traceback);

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
#endif
}

#ifdef FLEXT_THREADS
void pybase::thrworker(thr_params *p)
{
    FLEXT_ASSERT(p);
    pybase *th = (pybase *)p->cl;
	work_data *w = (work_data *)p->var->_ext;

	++th->thrcount; // \todo this should be atomic
    PyThreadState *state = PyLock();

    // call worker
	th->docall(w->fun,w->args);
	delete w;

    PyUnlock(state);
    --th->thrcount; // \todo this should be atomic
}

bool pybase::qucall(PyObject *fun,PyObject *args)
{
    FifoEl *el = qufifo.New();
    el->Set(this,fun,args);
    qufifo.Put(el);
    qucond.Signal();
    return true;
}

void pybase::quworker(thr_params *)
{
    FifoEl *el;
    PyThreadState *my = FindThreadState(),*state;

    for(;;) {
        while(el = qufifo.Get()) {
        	++el->th->thrcount; // \todo this should be atomic
            state = PyLock(my);
            el->th->docall(el->fun,el->args);
            Py_XDECREF(el->fun);
            Py_XDECREF(el->args);
            PyUnlock(state);
            --el->th->thrcount; // \todo this should be atomic
            qufifo.Free(el);
        }
        qucond.Wait();
    }

    // we never end
#if 0
    state = PyLock(my);
    // unref remaining Python objects
    while(el = qufifo.Get()) {
        Py_XDECREF(el->fun);
        Py_XDECREF(el->args);
        qufifo.Free(el);
    }
    PyUnlock(state);
#endif
}

void pybase::erasethreads()
{
    PyFifo tmp;
    FifoEl *el;
    while(el = qufifo.Get()) {
        if(el->th == this) {
            Py_XDECREF(el->fun);
            Py_XDECREF(el->args);
            qufifo.Free(el);
        }
        else
            tmp.Put(el);
    }
    // Push back
    while(el = tmp.Get()) qufifo.Put(el);
}
#endif

#if FLEXT_SYS == FLEXT_SYS_MAX
short pybase::patcher_myvol(t_patcher *x)
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

bool pybase::collect()
{
    if(gcollect) {
        PyObject *ret = PyObject_CallObject(gcollect,NULL);
        if(ret) {
#ifdef FLEXT_DEBUG
            int refs = PyInt_AsLong(ret);
            if(refs) post("py/pyext - Garbage collector reports %i unreachable objects",refs);
#endif
            Py_DECREF(ret);
            return false;
        }
    }
    return true;
}

/*
PY_EXPORT PyThreadState *py_Lock(PyThreadState *st = NULL) { return pybase::PyLock(st?st:pybase::FindThreadState()); }
PY_EXPORT PyThreadState *py_LockSys() { return pybase::PyLockSys(); }
PY_EXPORT void py_Unlock(PyThreadState *st) { pybase::PyUnlock(st); }
*/
