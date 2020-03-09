/*
py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2019 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __PYSYMBOL_H
#define __PYSYMBOL_H

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#ifdef PY_USE_FRAMEWORK
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
} pySymbol;

PY_EXPORT extern PyTypeObject pySymbol_Type;

PY_EXPORT extern pySymbol *pySymbol__;
PY_EXPORT extern pySymbol *pySymbol_bang;
PY_EXPORT extern pySymbol *pySymbol_list;
PY_EXPORT extern pySymbol *pySymbol_symbol;
PY_EXPORT extern pySymbol *pySymbol_float;
PY_EXPORT extern pySymbol *pySymbol_int;


#define pySymbol_Check(op) PyObject_TypeCheck(op, &pySymbol_Type)
#define pySymbol_CheckExact(op) ((op)->ob_type == &pySymbol_Type)


PY_EXPORT PyObject *pySymbol_FromSymbol(const t_symbol *sym);

inline PyObject *pySymbol_FromString(const char *str)
{
    return pySymbol_FromSymbol(flext::MakeSymbol(str));
}

inline PyObject *pySymbol_FromString(PyObject *str)
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
    
    return pySymbol_FromString(cstr);
}

inline const t_symbol *pySymbol_AS_SYMBOL(PyObject *op) 
{
    return ((pySymbol *)op)->sym;
}

inline const t_symbol *pySymbol_AsSymbol(PyObject *op) 
{
    return pySymbol_Check(op)?pySymbol_AS_SYMBOL(op):NULL;
}

inline const char *pySymbol_AS_STRING(PyObject *op) 
{
    return flext::GetString(pySymbol_AS_SYMBOL(op));
}

inline const t_symbol *pyObject_AsSymbol(PyObject *op)
{
#if PY_MAJOR_VERSION < 3
    if(PyString_Check(op))
        return flext::MakeSymbol(PyString_AS_STRING(op));
    else
#endif
    if(PyUnicode_Check(op))
        return flext::MakeSymbol(PyUnicode_AsUTF8(op));
    else
        return pySymbol_AsSymbol(op);
}

#endif
