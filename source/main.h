/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __MAIN_H
#define __MAIN_H

#include <flext.h>
#include <Python.h>

#if FLEXT_OS == FLEXT_LINUX || FLEXT_OS == FLEXT_IRIX
#include <unistd.h>
#endif

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#define PY__VERSION "0.1.1"


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

	static PyObject *MakePyArgs(const t_symbol *s,const AtomList &args,I inlet = -1,BL withself = false);
	static AtomList *GetPyArgs(PyObject *pValue,PyObject **self = NULL);

protected:

	V m_doc();

	PyObject *module,*dict; // inherited user class module and associated dictionary

	static I pyref;
	static const C *py_doc;

	V GetModulePath(const C *mod,C *dir,I len);
	V AddToPath(const C *dir);
	V SetArgs(I argc,const t_atom *argv);
	V ImportModule(const C *name);
	V ReloadModule();

	V Register(const C *reg);
	V Unregister(const C *reg);
	V Reregister(const C *reg);
	virtual V Reload() = 0;

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

	// ----thread stuff ------------

	V m_detach(BL det) { detach = det; }
	virtual V m_stop(int argc,const t_atom *argv);

	BL detach,shouldexit;
	I thrcount;
	t_clock *clk;
	I stoptick;

	static V tick(py *obj);

public:
	static PyInterpreterState *pystate;
	PyThreadState *pythrmain;

#ifdef FLEXT_THREADS
	ThrMutex mutex;
	V Lock() { mutex.Unlock(); }
	V Unlock() { mutex.Unlock(); }
#else
	V Lock() {}
	V Unlock() {}
#endif

protected:
	// callbacks

	FLEXT_CALLBACK_B(m_detach)
	FLEXT_CALLBACK_V(m_stop)
	FLEXT_CALLBACK(m_doc)
};

#ifdef FLEXT_THREADS
#define PY_LOCK \
	{ \
	PyThreadState *thrst = PyThreadState_New(pystate); \
	PyEval_AcquireThread(thrst); 

#define PY_UNLOCK \
    PyThreadState_Clear(thrst);        /* must have lock */ \
    PyEval_ReleaseThread(thrst);  \
    PyThreadState_Delete(thrst);       /* needn't have lock */ \
	}
#else
#define PY_LOCK 
#define PY_UNLOCK 
#endif

#endif
