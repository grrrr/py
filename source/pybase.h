/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __PYBASE_H
#define __PYBASE_H

#include "main.h"
#include "pysymbol.h"
#include "pybuffer.h"

class pybase
    : public flext
{
public:
	pybase();
	virtual ~pybase();

    void Exit();

	static PyObject *MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,int inlet = -1);
	static PyObject *MakePyArg(const t_symbol *s,int argc,const t_atom *argv);
	static const t_symbol *GetPyArgs(AtomList &lst,PyObject *pValue,int offs = 0);
	static const t_symbol *GetPyAtom(AtomList &lst,PyObject *pValue);

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

	AtomList args;

    void AddCurrentPath(flext_base *o);
	void GetModulePath(const char *mod,char *dir,int len);
	void AddToPath(const char *dir);
	void SetArgs();

    bool OutObject(flext_base *ext,int o,PyObject *obj);

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
	static bool IsAtom(const t_symbol *s) { return s == sym_float || s == sym_int || s == sym_symbol || s == sym_pointer; }

//	enum retval { nothing,atom,sequ };

	// --- module stuff -----

	static PyObject *module_obj,*module_dict;
	static PyObject *builtins_obj,*builtins_dict;
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
    bool pymsg;

    bool gencall(PyObject *fun,PyObject *args);

    bool docall(PyObject *fun,PyObject *args)
    {
        callpy(fun,args);
        if(PyErr_Occurred()) { 
            exchandle(); 
            return false; 
        }
        else 
            return true;
    }

//    virtual bool thrcall(void *data) = 0;
    virtual void callpy(PyObject *fun,PyObject *args) = 0;

    void exchandle();

    static bool collect();

protected:

#ifdef FLEXT_THREADS
    static void thrworker(thr_params *data); 

    bool qucall(PyObject *fun,PyObject *args);

    static void quworker(thr_params *);
    void erasethreads();

    static PyFifo qufifo;
    static ThrCond qucond;
    static PyThreadState *pythrsys;

    static PyThreadState *FindThreadState();
    static void FreeThreadState();
#else
    static PyThreadState *FindThreadState() { return NULL; }
#endif

    static const t_symbol *sym_fint; // float or int symbol, depending on native number message type

    static const t_symbol *getone(t_atom &at,PyObject *arg);
    static const t_symbol *getlist(t_atom *lst,PyObject *seq,int cnt,int offs = 0);

public:

#ifdef FLEXT_THREADS
	ThrMutex mutex;
	inline void Lock() { mutex.Unlock(); }
	inline void Unlock() { mutex.Unlock(); }

    // this is especially needed when one py/pyext object calls another one
    // we don't want the message to be queued, but otoh we have to avoid deadlock
    // (recursive calls can only happen in the system thread)
    static int lockcount;

	static PyThreadState *PyLock(PyThreadState *st = FindThreadState()) 
    { 
        if(!IsSystemThread() || !lockcount++) PyEval_AcquireLock();
	    return PyThreadState_Swap(st);
    }

	static PyThreadState *PyLockSys() 
    { 
        if(!lockcount++) PyEval_AcquireLock();
	    return PyThreadState_Swap(pythrsys);
    }

	static void PyUnlock(PyThreadState *st) 
    {
        PyThreadState *old = PyThreadState_Swap(st);
        if(old != pythrsys || !--lockcount) PyEval_ReleaseLock();
    }

#else
	inline void Lock() {}
	inline void Unlock() {}

	static PyThreadState *PyLock(PyThreadState * = NULL) { return NULL; }
	static PyThreadState *PyLockSys() { return NULL; }
	static void PyUnlock(PyThreadState *st) {}
#endif

	static PyObject* StdOut_Write(PyObject* Self, PyObject* Args);
};

#endif
