/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pysymbol.h"
#include "structmember.h"

static PyObject *symbol_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pySymbol *self = (pySymbol *)type->tp_alloc(type, 0);
    if(self) self->sym = flext::sym__;
    return (PyObject *)self;
}

static int symbol_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    FLEXT_ASSERT(pySymbol_Check(self));

    PyObject *arg = PySequence_GetItem(args,0); // new reference
    if(!arg) return -1;

    int ret = -1;

    if(pySymbol_Check(arg)) 
        ((pySymbol *)self)->sym = pySymbol_SYMBOL(arg);
    else if(PyString_Check(arg)) {
        ((pySymbol *)self)->sym = flext::MakeSymbol(PyString_AS_STRING(arg));
    }
    else {
        // better use repr or str?
        PyStringObject *str = (PyStringObject *)PyObject_Str(arg); // new reference
        if(str) {
            ((pySymbol *)self)->sym = flext::MakeSymbol(PyString_AS_STRING(str));
            Py_DECREF(str);
            ret = 0;
        }
    }
    Py_DECREF(arg);

    return ret;
}

static PyObject *symbol_str(PyObject *self)
{
    FLEXT_ASSERT(pySymbol_Check(self));
    return (PyObject *)PyString_FromString(pySymbol_AS_STRING(self));
}

static PyObject *symbol_repr(PyObject *self)
{
    FLEXT_ASSERT(pySymbol_Check(self));
    return (PyObject *)PyString_FromFormat("<py.Symbol %s>",pySymbol_AS_STRING(self));
}

PyTypeObject pySymbol_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Symbol",              /*tp_name*/
    sizeof(pySymbol),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    symbol_repr,               /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    symbol_str,                /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT /*| Py_TPFLAGS_BASETYPE*/,   /*tp_flags*/
    "Symbol objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)symbol_init,      /* tp_init */
    0,                         /* tp_alloc */
    symbol_new,                 /* tp_new */
};

void initsymbol()
{
    if(PyType_Ready(&pySymbol_Type) < 0)
        FLEXT_ASSERT(false);
    else
        Py_INCREF(&pySymbol_Type);
}
