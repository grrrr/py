/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyprefix.h"

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    const t_symbol *sym;
} pySymbol;

extern PyTypeObject pySymbol_Type;

extern pySymbol *pySymbol__;
extern pySymbol *pySymbol_bang;
extern pySymbol *pySymbol_list;
extern pySymbol *pySymbol_symbol;
extern pySymbol *pySymbol_float;
extern pySymbol *pySymbol_int;


#define pySymbol_Check(op) PyObject_TypeCheck(op, &pySymbol_Type)
#define pySymbol_CheckExact(op) ((op)->ob_type == &PySymbol_Type)


PyObject *pySymbol_FromSymbol(const t_symbol *sym);

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
    if(PyString_Check(op))
        return flext::MakeSymbol(PyString_AS_STRING(op));
    else
        return pySymbol_AsSymbol(op);
}

