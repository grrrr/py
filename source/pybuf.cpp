/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifdef PY_NUMARRAY

#include "main.h"
#include <numarray/numarray.h>


static bool nasupport = false;

void py::setupNumarray()
{
    import_libnumarray();
    if(PyErr_Occurred())
        // catch import error
        PyErr_Clear();
    else {
        // numarray support ok
        nasupport = true;
        post("");
	    post("Numarray support enabled");
    }
}

PyObject *py::py_import(PyObject *,PyObject *args)
{
    if(!nasupport) {
        PyErr_Format(PyExc_RuntimeError,"No numarray support");
        return NULL;
    }

    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);
    const t_symbol *bufsym;
    PyObject *ret = NULL;
    if(
        sz >= 1 && 
        (bufsym = pyObject_AsSymbol(PyTuple_GET_ITEM(args,0))) != NULL
    ) {
#ifdef FLEXT_THREADS
        flext::Lock();
#endif
        flext::buffer buf(bufsym);
        if(!buf.Ok() || !buf.Valid())
            PyErr_Format(PyExc_RuntimeError,"Buffer %s is not available",GetString(bufsym));
        else {
            // \todo for large frame counts do the copying in a number of steps
//            buf.Lock();
            ret = (PyObject *)NA_NewArray(buf.Data(),tFloat32,2,buf.Frames(),buf.Channels());
//            buf.Unlock();
        }
#ifdef FLEXT_THREADS
        flext::Unlock();
#endif
    }
    else
        PyErr_Format(PyExc_SyntaxError,"Syntax: _import [buffer]");

    return ret;
}

PyObject *py::py_export(PyObject *,PyObject *args)
{
    if(!nasupport) {
        PyErr_Format(PyExc_RuntimeError,"No numarray support");
        return NULL;
    }

    // should always be a tuple
    FLEXT_ASSERT(PyTuple_Check(args));

	const int sz = PyTuple_GET_SIZE(args);
    const t_symbol *bufsym;
    PyObject *ret = NULL;
    PyArrayObject *data = NULL;
    if(
        sz >= 2 && 
        (bufsym = pyObject_AsSymbol(PyTuple_GET_ITEM(args,0))) != NULL && 
        (data = NA_InputArray(PyTuple_GET_ITEM(args,1),tFloat32,C_ARRAY)) != NULL
    ) {
        if(data->nd > 2)
            PyErr_Format(PyExc_ValueError,"Input array is multi-dimensional");
        else {
#ifdef FLEXT_THREADS
            flext::Lock();
#endif
            flext::buffer buf(bufsym);
            if(!buf.Ok() || !buf.Valid())
                PyErr_Format(PyExc_RuntimeError,"Buffer %s is not available",GetString(bufsym));
            else {
                int sz = buf.Frames(),chns = buf.Channels();
                if(data->dimensions[0] < sz) sz = data->dimensions[0];

                if(data->nd == 1)
                    chns = 1;
                else if(data->dimensions[1] < chns) 
                    chns = data->dimensions[1];

    //            buf.Lock();
                flext::CopySamples(buf.Data(),(t_sample *)NA_OFFSETDATA(data),sz*chns);
    //            buf.Unlock();
                buf.Dirty(true);

                Py_INCREF(ret = Py_None);
            }
#ifdef FLEXT_THREADS
            flext::Unlock();
#endif
        }
    }
    else
        PyErr_Format(PyExc_SyntaxError,"Syntax: _export [buffer] [numarray]");

    Py_XDECREF(data);
    return ret;
}

#endif // PY_NUMARRAY
