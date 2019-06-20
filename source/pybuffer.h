/*
py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2019 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __PYBUFFER_H
#define __PYBUFFER_H

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#if FLEXT_OS == FLEXT_OS_MAC
#include <Python/Python.h>
#else
#include <Python.h>
#endif


#ifdef _MSC_VER
    #ifdef PY_EXPORTS
        #define PY_EXPORT __declspec(dllexport)
    #else
        #define PY_EXPORT __declspec(dllimport)
    #endif
#else
    #define PY_EXPORT
#endif

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    const t_symbol *sym;
    flext::buffer *buf;
    flext::buffer::lock_t lock;
    bool dirty;
} pySamplebuffer;

PY_EXPORT extern PyTypeObject pySamplebuffer_Type;

#define pySamplebuffer_Check(op) PyObject_TypeCheck(op, &pySamplebuffer_Type)
#define pySamplebuffer_CheckExact(op) ((op)->ob_type == &pySamplebuffer_Type)


PY_EXPORT PyObject *pySamplebuffer_FromSymbol(const t_symbol *sym);

inline PyObject *pySamplebuffer_FromString(const char *str)
{
    return pySamplebuffer_FromSymbol(flext::MakeSymbol(str));
}

inline PyObject *pySamplebuffer_FromString(PyObject *str)
{
    const char *cstr;
#if PY_MAJOR_VERSION < 3
    if(PyString_Check(str))
        cstr = PyString_AsString(str);
    else
#endif
    if(PyUnicode_Check(str))
        cstr = PyUnicode_AsUTF8(str);
    else
        PyErr_SetString(PyExc_TypeError, "Type must be string or unicode");
    return pySamplebuffer_FromString(cstr);
}

inline const t_symbol *pySamplebuffer_AS_SYMBOL(PyObject *op) 
{
    return ((pySamplebuffer *)op)->sym;
}

inline const t_symbol *pySamplebuffer_AsSymbol(PyObject *op) 
{
    return pySamplebuffer_Check(op)?pySamplebuffer_AS_SYMBOL(op):NULL;
}

inline const char *pySamplebuffer_AS_STRING(PyObject *op) 
{
    return flext::GetString(pySamplebuffer_AS_SYMBOL(op));
}

#endif
