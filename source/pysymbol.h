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

#define pySymbol_Check(op) PyObject_TypeCheck(op, &pySymbol_Type)
#define pySymbol_CheckExact(op) ((op)->ob_type == &PySymbol_Type)

inline const t_symbol *pySymbol_SYMBOL(PyObject *py) 
{
    return ((pySymbol *)py)->sym;
}

inline const t_symbol *pySymbol_Symbol(PyObject *py) 
{
    return pySymbol_Check(py)?pySymbol_SYMBOL(py):NULL;
}

inline const char *pySymbol_AS_STRING(PyObject *py) 
{
    return flext::GetString(pySymbol_SYMBOL(py));
}
