/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2004 Thomas Grill (gr@grrrr.org)
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

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 408)
#error You need at least flext version 0.4.8
#endif

#define PY__VERSION "0.2.0pre"


#define PYEXT_MODULE "pyext" // name for module
#define PYEXT_CLASS "_class"  // name for base class

#define PY_STOP_WAIT 1000  // ms
#define PY_STOP_TICK 10  // ms



#define I int
#define C char
#define V void
#define BL bool
#define F float
#define D double


#include "main.h"

class py:
	public flext_base
{
	FLEXT_HEADER(py,flext_base)

public:
	py();
	~py();
	static V lib_setup();

	static PyObject *MakePyArgs(const t_symbol *s,int argc,const t_atom *argv,I inlet = -1,BL withself = false);
	static AtomList *GetPyArgs(PyObject *pValue,PyObject **self = NULL);

protected:

    V m__dir(PyObject *obj);
	V m__doc(PyObject *obj);

    V m_dir() { m__dir(module); }
    V mg_dir(AtomList &lst) { m__dir(module); }
    V m_doc() { m__doc(dict); }

	PyObject *module,*dict; // inherited user class module and associated dictionary

	static const C *py_doc;

    V GetDir(PyObject *obj,AtomList &lst);

	V GetModulePath(const C *mod,C *dir,I len);
	V AddToPath(const C *dir);
	V SetArgs(I argc,const t_atom *argv);
	V ImportModule(const C *name);
	V UnimportModule();
	V ReloadModule();

	V Register(const C *reg);
	V Unregister(const C *reg);
	V Reregister(const C *reg);
	virtual V Reload() = 0;

    V Respond(BL b) { if(respond) { t_atom a[1]; SetBool(a[0],b); ToOutAnything(GetOutAttr(),MakeSymbol("response"),1,a); } }

	static BL IsAnything(const t_symbol *s) { return s && s != sym_bang && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer; }

	enum retval { nothing,atom,sequ /*,tuple,list*/ };

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
	static PyObject *py_inchannels(PyObject *,PyObject *args);
	static PyObject *py_outchannels(PyObject *,PyObject *args);
#if FLEXT_SYS == FLEXT_SYS_PD
	static PyObject *py_getvalue(PyObject *,PyObject *args);
	static PyObject *py_setvalue(PyObject *,PyObject *args);
#endif

	// ----thread stuff ------------

	virtual V m_stop(int argc,const t_atom *argv);

	BL detach,shouldexit,respond;
	I thrcount;
	I stoptick;
    Timer stoptmr;

	V tick(V *);

public:

#ifdef FLEXT_THREADS
	ThrMutex mutex;
	inline V Lock() { mutex.Unlock(); }
	inline V Unlock() { mutex.Unlock(); }
#else
	inline V Lock() {}
	inline V Unlock() {}
#endif

	static PyObject* StdOut_Write(PyObject* Self, PyObject* Args);

protected:
	// callbacks

	FLEXT_ATTRVAR_B(detach)
	FLEXT_ATTRVAR_B(respond)
	FLEXT_CALLBACK_V(m_stop)
	FLEXT_CALLBACK(m_dir)
	FLEXT_CALLGET_V(mg_dir)
	FLEXT_CALLBACK(m_doc)
    FLEXT_CALLBACK_T(tick)
};

#ifdef FLEXT_THREADS

PyThreadState *FindThreadState();
void FreeThreadState();

#define PY_LOCK \
	{ \
    PyEval_AcquireLock(); \
	PyThreadState *__st = FindThreadState(); \
	PyThreadState *__oldst = PyThreadState_Swap(__st);

#define PY_UNLOCK \
    PyThreadState_Swap(__oldst); \
    PyEval_ReleaseLock(); \
    }

#else
#define PY_LOCK 
#define PY_UNLOCK 
#endif

#endif
