/* 

pyext - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#ifndef __MAIN_H
#define __MAIN_H

#include <flext.h>
#include <Python.h>
#ifndef NT
#include <unistd.h>
#endif

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 302)
#error You need at least flext version 0.3.2
#endif

#define PY__VERSION "0.1.0pre"


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

protected:
	C *sName;
	I hName;

	class lookup {
	public:
		lookup(I hash,PyObject *mod,py *_th);
		~lookup();

		V Set(PyObject *mod,py *_th);
		V Add(lookup *l);
		py *GetThis(PyObject *mod);

		I modhash;
		PyObject *module,*dict;
		lookup *nxt;
		py *th;
	};

	static lookup *modules;
	static I pyref;

	static py *GetThis(PyObject *mod) { return modules?modules->GetThis(mod):NULL; }

	V GetModulePath(const C *mod,C *dir,I len);
	V AddToPath(const C *dir);
	V SetArgs(I argc,t_atom *argv);
	V ImportModule(const C *name);
	V SetModule(I hname,PyObject *module);
	V ReloadModule();
	PyObject *GetModule();
	PyObject *GetDict();
	PyObject *GetFunction(const C *func);
	static PyObject *MakePyArgs(const t_symbol *s,I argc,t_atom *argv,I inlet = -1);
	static t_atom *GetPyArgs(int &argc,PyObject *pValue,PyObject **self = NULL);

	static BL IsAnything(const t_symbol *s) { return s && s != sym_bang && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer; }

	static C *strdup(const C *s);

	enum retval { nothing,atom,tuple,list };

	// ----------------------------------

	V m_detach(BL det) { detach = det; }

	BL detach;
	I thrcount;

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
};

#ifdef FLEXT_THREADS
#define PY_LOCK \
	{ \
	PyEval_AcquireLock(); \
    PyThreadState *new_state = PyThreadState_New(pystate); /* must have lock */ \
    PyThreadState *prev_state = PyThreadState_Swap(new_state);

#define PY_UNLOCK \
	new_state = PyThreadState_Swap(prev_state); \
    PyThreadState_Clear(new_state);        /* must have lock */ \
    PyEval_ReleaseLock(); \
    PyThreadState_Delete(new_state);       /* needn't have lock */ \
	}

#else
#define PY_LOCK 
#define PY_UNLOCK 
#endif

#endif
