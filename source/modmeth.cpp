/* 

py/pyext - python external object for PD and Max/MSP

Copyright (c) 2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


// function table for module
PyMethodDef py::func_tbl[] = 
{
	{ "_send", py::py_send, METH_VARARGS,"Send message to a named object" },
#ifdef FLEXT_THREADS
	{ "_priority", py::py_priority, METH_VARARGS,"Set priority of current thread" },
#endif

	{ "_samplerate", py::py_samplerate, METH_NOARGS,"Get system sample rate" },
	{ "_blocksize", py::py_blocksize, METH_NOARGS,"Get system block size" },
	{ "_inchannels", py::py_inchannels, METH_NOARGS,"Get number of audio in channels" },
	{ "_outchannels", py::py_outchannels, METH_NOARGS,"Get number of audio out channels" },

#if FLEXT_SYS == FLEXT_SYS_PD
	{ "_getvalue", py::py_getvalue, METH_VARARGS,"Get value of a 'value' object" },
	{ "_setvalue", py::py_setvalue, METH_VARARGS,"Set value of a 'value' object" },
#endif

	{NULL, NULL, 0, NULL} // sentinel
};

const C *py::py_doc =
	"py/pyext - python external object for PD and Max/MSP, (C)2002-2004 Thomas Grill\n"
	"\n"
	"This is the pyext module. Available function:\n"
	"_send(args...): Send a message to a send symbol\n"
#ifdef FLEXT_THREADS
	"_priority(int): Raise/lower thread priority\n"
#endif
	"_samplerate(): Get system sample rate\n"
	"_blocksize(): Get current blocksize\n"
	"_inchannels(): Get number of audio in channels\n"
	"_outchannels(): Get number of audio out channels\n"
    "_getvalue(name): Get value of a 'value' object\n"
    "_setvalue(name,float): Set value of a 'value' object\n"
;



V py::tick(V *)
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
			post("%s - Threads couldn't be stopped entirely - %i remaining",thisName(),thrcount);
			shouldexit = false;
		}
		else
			// continue waiting
            stoptmr.Delay(PY_STOP_TICK/1000.);
	}

	Unlock();
}

V py::m_stop(int argc,const t_atom *argv)
{
	if(thrcount) {
		Lock();

		I wait = PY_STOP_WAIT;
		if(argc >= 1 && CanbeInt(argv[0])) wait = GetAInt(argv[0]);

		I ticks = wait/PY_STOP_TICK;
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
		
}

PyObject *py::py_samplerate(PyObject *self,PyObject *args)
{
	return PyFloat_FromDouble(sys_getsr());
}

PyObject *py::py_blocksize(PyObject *self,PyObject *args)
{
	return PyLong_FromLong(sys_getblksize());
}

PyObject *py::py_inchannels(PyObject *self,PyObject *args)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	I ch = sys_get_inchannels();
#elif FLEXT_SYS == FLEXT_SYS_MAX
	I ch = sys_getch(); // not working
#else
#pragma message("Not implemented!")
	ch = 0;
#endif
	return PyLong_FromLong(ch);
}

PyObject *py::py_outchannels(PyObject *self,PyObject *args)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	I ch = sys_get_outchannels();
#elif FLEXT_SYS == FLEXT_SYS_MAX
	I ch = sys_getch(); // not working
#else
#pragma message("Not implemented!")
	ch = 0;
#endif
	return PyLong_FromLong(ch);
}

PyObject *py::py_send(PyObject *,PyObject *args)
{
    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	PyObject *name = PyTuple_GetItem(args,0); // borrowed reference
    if(name && PyString_Check(name)) {
		const t_symbol *recv = MakeSymbol(PyString_AsString(name));
		int sz = PySequence_Size(args);
		PyObject *val;

		bool tp = 
            sz == 2 && 
            PySequence_Check(
                val = PyTuple_GetItem(args,1) // borrowed ref
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
PyObject *py::py_priority(PyObject *self,PyObject *args)
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
PyObject *py::py_getvalue(PyObject *self,PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

    PyObject *ret;
	PyObject *name = PyTuple_GetItem(args,0); // borrowed reference

    if(name && PyString_Check(name)) {
		const t_symbol *sym = MakeSymbol(PyString_AsString(name));

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

PyObject *py::py_setvalue(PyObject *self,PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

	PyObject *name = PyTuple_GetItem(args,0); // borrowed reference
	PyObject *val = PyTuple_GetItem(args,1); // borrowed reference
    if(name && val && PyString_Check(name) && PyNumber_Check(val)) {
		const t_symbol *sym = MakeSymbol(PyString_AsString(name));
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


