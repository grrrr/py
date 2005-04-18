/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __MAIN_H
#define __MAIN_H

#include "pyprefix.h"
#include "pysymbol.h"
#include "pybuffer.h"
#include <flcontainers.h>
#include <string>

#if FLEXT_OS == FLEXT_LINUX || FLEXT_OS == FLEXT_IRIX
#include <unistd.h>
#endif

#if FLEXT_SYS == FLEXT_SYS_PD && (!defined (PD_MINOR_VERSION) || PD_MINOR_VERSION < 37)
#error PD version >= 0.37 required, please upgrade! 
#endif

#define PY__VERSION "0.2.0pre"


#define PYEXT_MODULE "pyext" // name for module
#define PYEXT_CLASS "_class"  // name for base class

#define REGNAME "_registry"

#define PY_STOP_WAIT 100  // ms
#define PY_STOP_TICK 1  // ms



class FifoEl
    : public Fifo::Cell
{
public:
    void Set(PyObject *f,PyObject *a) { fun = f,args = a; }
    PyObject *fun,*args;
};

typedef PooledFifo<FifoEl> PyFifo;


class pybase
    : public flext
{
public:
	pybase();
	virtual ~pybase();

    void Exit();

	static PyObject *MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,int inlet = -1,bool withself = false);
	static bool GetPyArgs(AtomList &lst,PyObject *pValue,int offs = 0,PyObject **self = NULL);

    static void lib_setup();

protected:

    virtual void DumpOut(const t_symbol *sym,int argc,const t_atom *argv) = 0;

    void m__dir(PyObject *obj);
	void m__doc(PyObject *obj);

    void m_dir() { m__dir(module); }
    void mg_dir(AtomList &lst) { m__dir(module); }
    void m_doc() { m__doc(dict); }

    std::string modname; // module name
	PyObject *module,*dict; // object module and associated dictionary

	static const char *py_doc;

    void GetDir(PyObject *obj,AtomList &lst);

	AtomListStatic<16> args;

	void GetModulePath(const char *mod,char *dir,int len);
	void AddToPath(const char *dir);
	void SetArgs();

    // reload module and all connected objects
    void Reload();

	bool ImportModule(const char *name);
	void UnimportModule();
	bool ReloadModule();

    // Get module registry
	PyObject *GetRegistry(const char *regname);
    // Set module registry
	void SetRegistry(const char *regname,PyObject *reg);

    // Register object
	void Register(PyObject *reg);
    // Unregister object
	void Unregister(PyObject *reg);

	virtual void LoadModule() = 0;
	virtual void UnloadModule() = 0;

	virtual void Load() = 0;
	virtual void Unload() = 0;

    void OpenEditor();
    void Respond(bool b);

    void Report() { while(PyErr_Occurred()) PyErr_Print(); }

	static bool IsAnything(const t_symbol *s) { return s && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer; }

	enum retval { nothing,atom,sequ };

	// --- module stuff -----

	static PyObject *module_obj,*module_dict;
	static PyMethodDef func_tbl[];

	static PyObject *py__doc__(PyObject *,PyObject *args);
	static PyObject *py_send(PyObject *,PyObject *args);
#ifdef FLEXT_THREADS
	static PyObject *py_priority(PyObject *,PyObject *args);
#endif

	static PyObject *py_arraysupport(PyObject *,PyObject *args);
	static PyObject *py_samplerate(PyObject *,PyObject *args);
	static PyObject *py_blocksize(PyObject *,PyObject *args);

#if FLEXT_SYS == FLEXT_SYS_PD
	static PyObject *py_getvalue(PyObject *,PyObject *args);
	static PyObject *py_setvalue(PyObject *,PyObject *args);
#endif

#ifdef PY_NUMARRAY
    static void setupNumarray();
	static PyObject *py_import(PyObject *,PyObject *args);
	static PyObject *py_export(PyObject *,PyObject *args);
#endif

	// ----thread stuff ------------

	virtual void m_stop(int argc,const t_atom *argv);

	bool respond;
#ifdef FLEXT_THREADS
    bool shouldexit;
	int thrcount;
	int stoptick;
    Timer stoptmr;

	void tick(void *);
#endif

    int detach;

    bool gencall(PyObject *fun,PyObject *args);
    virtual bool thrcall(void *data) = 0;
    virtual bool callpy(PyObject *fun,PyObject *args) = 0;

#if FLEXT_SYS == FLEXT_SYS_MAX
    static short patcher_myvol(t_patcher *x);
#endif

    static bool collect();

protected:

	void work_wrapper(void *data); 

#ifdef FLEXT_THREADS
    bool qucall(PyObject *fun,PyObject *args);
    void threadworker();
    PyFifo qufifo;
    ThrCond qucond;
    static PyThreadState *pythrsys;

    static PyThreadState *FindThreadState();
    static void FreeThreadState();
#else
    static PyThreadState *FindThreadState() { return NULL; }
#endif

public:

#ifdef FLEXT_THREADS
	ThrMutex mutex;
	inline void Lock() { mutex.Unlock(); }
	inline void Unlock() { mutex.Unlock(); }

    // this is especially needed when one py/pyext object calls another one
    // we don't want the message to be queued, but otoh we have to avoid deadlock
    // (recursive calls can only happen in the system thread)
    static int lockcount;

	inline PyThreadState *PyLock(PyThreadState *st = FindThreadState()) 
    { 
        if(!IsSystemThread() || !lockcount++) PyEval_AcquireLock();
	    return PyThreadState_Swap(st);
    }

	inline PyThreadState *PyLockSys() 
    { 
        if(!lockcount++) PyEval_AcquireLock();
	    return PyThreadState_Swap(pythrsys);
    }

	inline void PyUnlock(PyThreadState *st) 
    {
        PyThreadState *old = PyThreadState_Swap(st);
        if(old != pythrsys || !--lockcount) PyEval_ReleaseLock();
    }

#else
	inline void Lock() {}
	inline void Unlock() {}

	inline PyThreadState *PyLock(PyThreadState * = NULL) { return NULL; }
	inline PyThreadState *PyLockSys() { return NULL; }
	inline void PyUnlock(PyThreadState *st) {}
#endif

	static PyObject* StdOut_Write(PyObject* Self, PyObject* Args);
};

#endif
