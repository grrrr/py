/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
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
#endif


PyObject *pybase::module_obj = NULL;
PyObject *pybase::module_dict = NULL;


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

	// -------------------------------------------------------------

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pyext);
	FLEXT_DSP_SETUP(pydsp);

#ifdef FLEXT_THREADS
    // release global lock
    PyEval_ReleaseLock();
#endif

	post("------------------------------------------------");
    post("");
}

FLEXT_LIB_SETUP(py,pybase::lib_setup)


pybase::pybase()
    : module(NULL),detach(0)
#ifdef FLEXT_THREADS
    , shouldexit(false),thrcount(0),stoptick(0)
#endif
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
            AtomList *l = GetPyArgs(pvar);
            if(l) { 
                lst = *l; delete l; 
            }
            else
                post("py/pyext - Argument list could not be created");
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
					int l = nl-s;
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

void pybase::SetArgs(int argc,const t_atom *argv)
{
	// script arguments
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

void pybase::ImportModule(const char *name)
{
	if(!name) return;

	module = PyImport_ImportModule((char *)name);  // increases module_obj ref count by one
	if(!module) {
		PyErr_Print();
		dict = NULL;
	}
	else
		dict = PyModule_GetDict(module);
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

void pybase::ReloadModule()
{
	if(module) {
		PyObject *newmod = PyImport_ReloadModule(module);
		if(!newmod) {
			PyErr_Print();
			// old module still exists?!
//			dict = NULL;
		}
		else {
			Py_XDECREF(module);
			module = newmod;
			dict = PyModule_GetDict(module); // borrowed
		}
	}
	else 
		post("py/pyext - No module to reload");
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

static const t_symbol *sym_response = flext::MakeSymbol("response");

void pybase::Respond(bool b) 
{ 
    if(respond) { 
        t_atom a; 
        SetBool(a,b); 
        DumpOut(sym_response,1,&a); 
    } 
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
            ret = callpy(pmeth,pargs);
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
			    ret = thrcall(new work_data(pmeth,pargs));
			    if(!ret) post("py/pyext - Failed to launch thread!");
		    }
            break;
#endif
        default:
            post("py/pyext - Unknown detach mode");
    }
    return ret;
}

void pybase::work_wrapper(void *data)
{
    FLEXT_ASSERT(data);

#ifdef FLEXT_THREADS
	++thrcount;
#endif

    PyThreadState *state = PyLock();

    // call worker
	work_data *w = (work_data *)data;
	callpy(w->fun,w->args);
	delete w;

    PyUnlock(state);

#ifdef FLEXT_THREADS
    --thrcount;
#endif
}

#ifdef FLEXT_THREADS
bool pybase::qucall(PyObject *fun,PyObject *args)
{
    FifoEl *el = qufifo.New();
    el->Set(fun,args);
    qufifo.Put(el);
    qucond.Signal();
    return true;
}

void pybase::threadworker()
{
    FifoEl *el;
    PyThreadState *my = FindThreadState(),*state;

   	++thrcount;
    for(;;) {
        while(el = qufifo.Get()) {
        	++thrcount;
            state = PyLock(my);
            callpy(el->fun,el->args);
            Py_XDECREF(el->fun);
            Py_XDECREF(el->args);
            PyUnlock(state);
            qufifo.Free(el);
            --thrcount;
        }
        if(shouldexit) 
            break;
        else
            qucond.Wait();
    }

    state = PyLock(my);
    // unref remaining Python objects
    while(el = qufifo.Get()) {
        Py_XDECREF(el->fun);
        Py_XDECREF(el->args);
        qufifo.Free(el);
    }
    PyUnlock(state);
    --thrcount;
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
