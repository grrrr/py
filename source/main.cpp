/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2004 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <map>

static PyMethodDef StdOut_Methods[] =
{
	{ "write", py::StdOut_Write, 1 },
	{ NULL,    NULL,           }  
};


#ifdef FLEXT_THREADS
typedef std::map<flext::thrid_t,PyThreadState *> PyThrMap;

static PyInterpreterState *pystate = NULL;
static PyThreadState *pythrmain = NULL;
static PyThrMap pythrmap;

PyThreadState *FindThreadState()
{
    flext::thrid_t id = flext::GetThreadId();
	PyThrMap::iterator it = pythrmap.find(id);
    if(it == pythrmap.end()) {
        // Make new thread state
        PyThreadState *st = PyThreadState_New(pystate);
        pythrmap[id] = st;
        return st;
    }
    else 
        return it->second;
}

void FreeThreadState()
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


V py::lib_setup()
{
	post("");
	post("py/pyext %s - python script objects, (C)2002-2004 Thomas Grill",PY__VERSION);
#ifdef FLEXT_DEBUG
	post("DEBUG version compiled on %s %s",__DATE__,__TIME__);
#endif
    post("");

	// -------------------------------------------------------------

	Py_Initialize();

#ifdef FLEXT_DEBUG
//	Py_DebugFlag = 1;
	Py_VerboseFlag = 1;
#endif

#ifdef FLEXT_THREADS
    // enable thread support and acquire the global thread lock
	PyEval_InitThreads();

    // get thread state
    pythrmain = PyThreadState_Get();
    // get main interpreter state
	pystate = pythrmain->interp;

    // add thread state of main thread to map
    pythrmap[GetThreadId()] = pythrmain;
#endif

    // register/initialize pyext module only once!
	module_obj = Py_InitModule(PYEXT_MODULE, func_tbl);
	module_dict = PyModule_GetDict(module_obj); // borrowed reference

	PyModule_AddStringConstant(module_obj,"__doc__",(C *)py_doc);

	// redirect stdout
	PyObject* py_out = Py_InitModule("stdout", StdOut_Methods);
	PySys_SetObject("stdout", py_out);

#ifdef FLEXT_THREADS
    // release global lock
    PyEval_ReleaseLock();
#endif

	// -------------------------------------------------------------

	FLEXT_SETUP(pyobj);
	FLEXT_SETUP(pyext);
}

FLEXT_LIB_SETUP(py,py::lib_setup)


PyObject *py::module_obj = NULL;
PyObject *py::module_dict = NULL;


py::py(): 
	module(NULL),
	detach(false),shouldexit(false),thrcount(0),
	stoptick(0)
{
	PY_LOCK
	Py_INCREF(module_obj);
	PY_UNLOCK

    FLEXT_ADDTIMER(stoptmr,tick);
}

py::~py()
{
	if(thrcount) {
		shouldexit = true;

		// Wait for a certain time
		for(int i = 0; i < (PY_STOP_WAIT/PY_STOP_TICK) && thrcount; ++i) Sleep((F)(PY_STOP_TICK/1000.));

		// Wait forever
		post("%s - Waiting for thread termination!",thisName());
		while(thrcount) Sleep(0.2f);
		post("%s - Okay, all threads have terminated",thisName());
	}
		
   	Py_XDECREF(module_obj);
}


void py::GetDir(PyObject *obj,AtomList &lst)
{
    if(obj) {
        PY_LOCK
    
        PyObject *pvar  = PyObject_Dir(obj);
	    if(!pvar)
		    PyErr_Print(); // no method found
	    else {
            AtomList *l = GetPyArgs(pvar);
            if(l) { 
                lst = *l; delete l; 
            }
            else
                post("%s - %s: List could not be created",thisName(),GetString(thisTag()));
            Py_DECREF(pvar);
        }

        PY_UNLOCK
    }
}

void py::m__dir(PyObject *obj)
{
    AtomList lst;
    GetDir(obj,lst);
    // dump dir to attribute outlet
    ToOutAnything(GetOutAttr(),thisTag(),lst.Count(),lst.Atoms());
}

V py::m__doc(PyObject *obj)
{
    if(obj) {
        PY_LOCK

		PyObject *docf = PyDict_GetItemString(obj,"__doc__"); // borrowed!!!
		if(docf && PyString_Check(docf)) {
			post("");
			const char *s = PyString_AsString(docf);

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

        PY_UNLOCK
	}
}


V py::SetArgs(I argc,const t_atom *argv)
{
	// script arguments
	C **sargv = new C *[argc+1];
	for(int i = 0; i <= argc; ++i) {
		sargv[i] = new C[256];
		if(!i) 
			strcpy(sargv[i],thisName());
		else
			GetAString(argv[i-1],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc+1,sargv);

	for(int j = 0; j <= argc; ++j) delete[] sargv[j];
	delete[] sargv;
}

V py::ImportModule(const C *name)
{
	if(!name) return;

	module = PyImport_ImportModule((C *)name);  // increases module_obj ref count by one
	if (!module) {

		PyErr_Print();
		dict = NULL;
	}
	else
		dict = PyModule_GetDict(module);
}

V py::UnimportModule()
{
	if(!module) return;

	assert(dict && module_obj && module_dict);

	Py_DECREF(module);

	// reference count to module is not 0 here, altough probably the last instance was unloaded
	// Python retains one reference to the module all the time 
	// we don't care

	module = NULL;
	dict = NULL;
}

V py::ReloadModule()
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
		post("%s - No module to reload",thisName());
}

V py::GetModulePath(const C *mod,C *dir,I len)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	// uarghh... pd doesn't show its path for extra modules

	C *name;
	I fd = open_via_path("",mod,".py",dir,&name,len,0);
	if(fd > 0) close(fd);
	else name = NULL;

	// if dir is current working directory... name points to dir
	if(dir == name) strcpy(dir,".");
#elif FLEXT_SYS == FLEXT_SYS_MAX
	// how do i get the path in Max/MSP?
    short path;
    long type;
    char smod[256];
    strcat(strcpy(smod,mod),".py");
    if(!locatefile_extended(smod,&path,&type,&type,-1))
        path_topathname(path,NULL,dir);
    else 
        // not found
        *dir = 0;
#else
	*dir = 0;
#endif
}

V py::AddToPath(const C *dir)
{
	if(dir && *dir) {
		PyObject *pobj = PySys_GetObject("path");
		if(pobj && PyList_Check(pobj)) {
			int i,n = PyList_Size(pobj);
			for(i = 0; i < n; ++i) {
				PyObject *pt = PyList_GetItem(pobj,i); // borrowed reference
				if(PyString_Check(pt) && !strcmp(dir,PyString_AS_STRING(pt))) break;
			}
			if(i == n) { // string is not yet existent in path
				PyObject *ps = PyString_FromString(dir);
				PyList_Append(pobj,ps);
			}
		}
		PySys_SetObject("path",pobj);
	}
}

static PyObject *output = NULL;

// post to the console
PyObject* py::StdOut_Write(PyObject* self, PyObject* args)
{
    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	int sz = PyTuple_Size(args);
	for(int i = 0; i < sz; ++i) {
		PyObject *val = PyTuple_GetItem(args,i); // borrowed reference
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
