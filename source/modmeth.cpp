#include "main.h"

V py::tick(py *th)
{
	th->Lock();

	if(!th->thrcount) {
		// all threads have stopped
		th->shouldexit = false;
		th->stoptick = 0;
	}
	else {
		// still active threads 
		if(!--th->stoptick) {
			post("%s - Threads couldn't be stopped entirely - %i remaining",th->thisName(),th->thrcount);
			th->shouldexit = false;
		}
		else
			// continue waiting
			clock_delay(th->clk,STOP_TICK);
	}

	th->Unlock();
}

V py::m_stop(int argc,t_atom *argv)
{
	if(thrcount) {
		Lock();

		I wait = STOP_WAIT;
		if(argc >= 1 && CanbeInt(argv[0])) wait = GetAInt(argv[0]);

		I ticks = wait/STOP_TICK;
		if(stoptick) {
			// already stopping
			if(ticks < stoptick) stoptick = ticks;
		}
		else
			stoptick = ticks;
		shouldexit = true;
		clock_delay(clk,STOP_TICK);

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
#ifdef PD
	I ch = sys_get_inchannels();
#else // MAXMSP
	I ch = sys_getch(); // not functioning
#endif
	return PyLong_FromLong(ch);
}

PyObject *py::py_outchannels(PyObject *self,PyObject *args)
{
#ifdef PD
	I ch = sys_get_outchannels();
#else // MAXMSP
	I ch = sys_getch(); // not functioning
#endif
	return PyLong_FromLong(ch);
}

PyObject *py::py_send(PyObject *,PyObject *args)
{
	const char *name;
    PyObject *val;
    if(!PyArg_ParseTuple(args, "sO:py_send", &name,&val)) {
        // handle error
		error("py/pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }

	const t_symbol *recv = MakeSymbol(name);
	
	I argc;
	t_atom *lst = GetPyArgs(argc,val);
	if(argc && lst) {
		t_class **cl = (t_class **)GetBound(recv);
		if(cl) {
#ifdef PD
			pd_forwardmess(cl,argc,lst);
#else
			#pragma message ("Send is not implemented")
#endif
		}
#ifdef _DEBUG
		else 
			post("py/pyext - Receiver doesn't exist");
#endif
	}
	else 
		post("py/pyext - No data to send");
	if(lst) delete[] lst;

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef FLEXT_THREADS
PyObject *py::py_priority(PyObject *self,PyObject *args)
{
	int val;
    if(!PyArg_ParseTuple(args, "i:py_priority", &val)) {
        // handle error
		error("py/pyext - INTERNAL ERROR, file %s - line %i",__FILE__,__LINE__);
    }
	else
		ChangePriority(val);

    Py_INCREF(Py_None);
    return Py_None;
}
#endif
