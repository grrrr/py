/* 

py/pyext - python external object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


// function table for module
PyMethodDef pybase::func_tbl[] = 
{
	{ "_send", pybase::py_send, METH_VARARGS,"Send message to a named object" },
#ifdef FLEXT_THREADS
	{ "_priority", pybase::py_priority, METH_VARARGS,"Set priority of current thread" },
#endif

	{ "_samplerate", pybase::py_samplerate, METH_NOARGS,"Get system sample rate" },
	{ "_blocksize", pybase::py_blocksize, METH_NOARGS,"Get system block size" },

#if FLEXT_SYS == FLEXT_SYS_PD
	{ "_getvalue", pybase::py_getvalue, METH_VARARGS,"Get value of a 'value' object" },
	{ "_setvalue", pybase::py_setvalue, METH_VARARGS,"Set value of a 'value' object" },
#endif
	{NULL, NULL, 0, NULL} // sentinel
};

const char *pybase::py_doc =
	"py/pyext - python external object for PD and Max/MSP, (C)2002-2005 Thomas Grill\n"
	"\n"
	"This is the pyext module. Available function:\n"
	"_send(args...): Send a message to a send symbol\n"
#ifdef FLEXT_THREADS
	"_priority(int): Raise/lower thread priority\n"
#endif
	"_samplerate(): Get system sample rate\n"
	"_blocksize(): Get current blocksize\n"
    "_getvalue(name): Get value of a 'value' object\n"
    "_setvalue(name,float): Set value of a 'value' object\n"
;


#ifdef FLEXT_THREADS
void pybase::tick(void *)
{
	Lock();

	if(!thrcount) {
		// all threads have stopped
		shouldexit = false;
		stoptick = 0;
	}
	else {
		// still active threads 
		if(!--stoptick) {
			post("py/pyext - Threads couldn't be stopped entirely - %i remaining",thrcount);
			shouldexit = false;
		}
		else
			// continue waiting
            stoptmr.Delay(PY_STOP_TICK/1000.);
	}

	Unlock();
}
#endif

void pybase::m_stop(int argc,const t_atom *argv)
{
#ifdef FLEXT_THREADS
	if(thrcount) {
		Lock();

		int wait = PY_STOP_WAIT;
		if(argc >= 1 && CanbeInt(argv[0])) wait = GetAInt(argv[0]);

		int ticks = wait/PY_STOP_TICK;
		if(stoptick) {
			// already stopping
			if(ticks < stoptick) stoptick = ticks;
		}
		else
			stoptick = ticks;
		shouldexit = true;
        stoptmr.Delay(PY_STOP_TICK/1000.);

		Unlock();
	}
#endif		
}

PyObject *pybase::py_samplerate(PyObject *self,PyObject *args)
{
	return PyFloat_FromDouble(sys_getsr());
}

PyObject *pybase::py_blocksize(PyObject *self,PyObject *args)
{
	return PyLong_FromLong(sys_getblksize());
}

PyObject *pybase::py_send(PyObject *,PyObject *args)
{
    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);

    const t_symbol *recv;
    if(
        sz >= 1 && 
        (recv = pyObject_AsSymbol(PyTuple_GET_ITEM(args,0))) != NULL
    ) {
		PyObject *val;

		bool tp = 
            sz == 2 && 
            PySequence_Check(
                val = PyTuple_GET_ITEM(args,1) // borrowed ref
            );

		if(!tp)
			val = PySequence_GetSlice(args,1,sz);  // new ref

		AtomList *lst = GetPyArgs(val);
		if(lst) {
			bool ok;
			if(lst->Count() && IsSymbol((*lst)[0]))
				ok = Forward(recv,GetSymbol((*lst)[0]),lst->Count()-1,lst->Atoms()+1);
			else
				ok = Forward(recv,*lst);

#ifdef FLEXT_DEBUG
            if(!ok)
				post("py/pyext - Receiver doesn't exist");
#endif
		}
		else 
			post("py/pyext - No data to send");
		if(lst) delete lst;

		if(!tp) Py_DECREF(val);
	}
	else
		post("py/pyext - Send name is invalid");

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef FLEXT_THREADS
PyObject *pybase::py_priority(PyObject *self,PyObject *args)
{
	int val;
    if(!PyArg_ParseTuple(args, "i:py_priority", &val)) {
		post("py/pyext - Syntax: _priority [int]");
    }
	else
		RelPriority(val);

    Py_INCREF(Py_None);
    return Py_None;
}
#endif

#if FLEXT_SYS == FLEXT_SYS_PD
PyObject *pybase::py_getvalue(PyObject *self,PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);
    const t_symbol *sym;
    PyObject *ret;

    if(
        sz == 1 && 
        (sym = pyObject_AsSymbol(PyTuple_GET_ITEM(args,0))) != NULL
    ) {
        float f;
        if(value_getfloat(const_cast<t_symbol *>(sym),&f)) {
		    post("py/pyext - Could not get value '%s'",GetString(sym));
            Py_INCREF(ret = Py_None);
        }
        else
            ret = PyFloat_FromDouble(f);
    }
    else {
        post("py/pyext - Syntax: _getvalue [name]");
        Py_INCREF(ret = Py_None);
    }
    return ret;
}

PyObject *pybase::py_setvalue(PyObject *self,PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);
	const t_symbol *sym;
	PyObject *val; // borrowed reference

    if(
        sz == 2 &&
        (sym = pyObject_AsSymbol(PyTuple_GET_ITEM(args,0))) != NULL &&
        PyNumber_Check(val = PyTuple_GET_ITEM(args,1))
    ) {
        float f = (float)PyFloat_AsDouble(val);

        if(value_setfloat(const_cast<t_symbol *>(sym),f))
		    post("py/pyext - Could not set value '%s'",GetString(sym));
    }
    else
        post("py/pyext - Syntax: _setvalue [name] [value]");

    Py_INCREF(Py_None);
    return Py_None;
}
#endif
