/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __MAIN_H
#define __MAIN_H

#define FLEXT_ATTRIBUTES 1

#include <flext.h>
#if FLEXT_OS == FLEXT_OS_MAC
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#if FLEXT_OS == FLEXT_LINUX || FLEXT_OS == FLEXT_IRIX
#include <unistd.h>
#endif

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#define PY__VERSION "0.2.0pre"


#define PYEXT_MODULE "pyext" // name for module
#define PYEXT_CLASS "_class"  // name for base class

#define PY_STOP_WAIT 1000  // ms
#define PY_STOP_TICK 10  // ms


class Fifo
{
protected:
    struct FifoEl {
        PyObject *fun;
        PyObject *args;
        FifoEl *nxt;
    };
public:
    Fifo(): head(NULL),tail(NULL) {}
    ~Fifo();

    bool Push(PyObject *f,PyObject *a);
    bool Pop(PyObject *&f,PyObject *&a);
    
protected:
    FifoEl *head,*tail;
};


class py:
	public flext_base
{
	FLEXT_HEADER(py,flext_base)

public:
	py();
	~py();
	static void lib_setup();

	static PyObject *MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,int inlet = -1,bool withself = false);
	static AtomList *GetPyArgs(PyObject *pValue,PyObject **self = NULL);

protected:

    void m__dir(PyObject *obj);
	void m__doc(PyObject *obj);

    void m_dir() { m__dir(module); }
    void mg_dir(AtomList &lst) { m__dir(module); }
    void m_doc() { m__doc(dict); }

	PyObject *module,*dict; // inherited user class module and associated dictionary

	static const char *py_doc;

    void GetDir(PyObject *obj,AtomList &lst);

	void GetModulePath(const char *mod,char *dir,int len);
	void AddToPath(const char *dir);
	void SetArgs(int argc,const t_atom *argv);
	void ImportModule(const char *name);
	void UnimportModule();
	void ReloadModule();

	void Register(const char *reg);
	void Unregister(const char *reg);
	void Reregister(const char *reg);
	virtual void Reload() = 0;

    void Respond(bool b);

	static bool IsAnything(const t_symbol *s) { return s && s != sym_bang && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer; }

	enum retval { nothing,atom,sequ };

	// --- module stuff -----

	static PyObject *module_obj,*module_dict;
	static PyMethodDef func_tbl[];

	static PyObject *py__doc__(PyObject *,PyObject *args);
	static PyObject *py_send(PyObject *,PyObject *args);
#ifdef FLEXT_THREADS
	static PyObject *py_priority(PyObject *,PyObject *args);
#endif

	static PyObject *py_samplerate(PyObject *,PyObject *args);
	static PyObject *py_blocksize(PyObject *,PyObject *args);

#if FLEXT_SYS == FLEXT_SYS_PD
	static PyObject *py_getvalue(PyObject *,PyObject *args);
	static PyObject *py_setvalue(PyObject *,PyObject *args);
#endif

	// ----thread stuff ------------

	virtual void m_stop(int argc,const t_atom *argv);

	bool shouldexit,respond;
	int thrcount;
	int stoptick;
    Timer stoptmr;
    int detach;

	void tick(void *);
    
    bool gencall(PyObject *fun,PyObject *args);
    virtual bool callpy(PyObject *fun,PyObject *args) = 0;

#if FLEXT_SYS == FLEXT_SYS_MAX
    static short patcher_myvol(t_patcher *x);
#endif

private:
    bool qucall(PyObject *fun,PyObject *args);
    void threadworker();
    Fifo qufifo;
    ThrCond qucond;

	void work_wrapper(void *data); 

#ifdef FLEXT_THREADS
	FLEXT_THREAD_X(work_wrapper)
#else
	FLEXT_CALLBACK_X(work_wrapper)
#endif

#ifdef FLEXT_THREADS
    static PyThreadState *FindThreadState();
    static void FreeThreadState();
#endif

public:

#ifdef FLEXT_THREADS
	ThrMutex mutex;
	inline void Lock() { mutex.Unlock(); }
	inline void Unlock() { mutex.Unlock(); }

	inline PyThreadState *PyLock() 
    { 
        PyEval_AcquireLock();
	    return PyThreadState_Swap(FindThreadState());
    }

	inline void PyUnlock(PyThreadState *st) 
    {
        PyThreadState_Swap(st);
        PyEval_ReleaseLock();
    }
#else
	inline void Lock() {}
	inline void Unlock() {}

	inline PyThreadState *PyLock() { return NULL; }
	inline void PyUnlock(PyThreadState *st) {}
#endif

	static PyObject* StdOut_Write(PyObject* Self, PyObject* Args);

protected:
	// callbacks

	FLEXT_ATTRVAR_I(detach)
	FLEXT_ATTRVAR_B(respond)
	FLEXT_CALLBACK_V(m_stop)
	FLEXT_CALLBACK(m_dir)
	FLEXT_CALLGET_V(mg_dir)
	FLEXT_CALLBACK(m_doc)
    FLEXT_CALLBACK_T(tick)

    FLEXT_THREAD(threadworker)
};

#endif
