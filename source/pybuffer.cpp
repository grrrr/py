/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"

#ifdef PY_NUMARRAY
#include <numarray/numarray.h>
static bool nasupport = false;
static NumarrayType numtype;
#endif

// PD defines a T_OBJECT symbol
#undef T_OBJECT
#include "structmember.h"
#include "bufferobject.h"

static PyObject *buffer_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pySamplebuffer *self = (pySamplebuffer *)pySamplebuffer_Type.tp_alloc(&pySamplebuffer_Type, 0);
    self->sym = NULL;
    self->buf = NULL;
    self->dirty = false;
    return (PyObject *)self;
}

static void buffer_dealloc(PyObject *obj)
{
    pySamplebuffer *self = (pySamplebuffer *)obj;

    if(self->buf) {
        self->buf->Unlock(self->lock);
        if(self->dirty) self->buf->Dirty(true);
        delete self->buf;
    }

    obj->ob_type->tp_free(obj);
}

static int buffer_init(PyObject *obj, PyObject *args, PyObject *kwds)
{
    FLEXT_ASSERT(pySamplebuffer_Check(obj));

    PyObject *arg = PySequence_GetItem(args,0); // new reference
    if(!arg) return -1;

    int ret = 0;

    pySamplebuffer *self = (pySamplebuffer *)obj;
    FLEXT_ASSERT(!self->sym && !self->buf);

    if(pySymbol_Check(arg))
        self->sym = pySymbol_AS_SYMBOL(arg);
    else if(PyString_Check(arg))
        self->sym = flext::MakeSymbol(PyString_AS_STRING(arg));
    else
        ret = -1;
    Py_DECREF(arg);

    if(self->sym) {
        flext::buffer *b = new flext::buffer(self->sym);
        if(b->Ok() && b->Valid())
            self->lock = (self->buf = b)->Lock();
        else
            delete b;
    }

    return ret;
}

static PyObject *buffer_repr(PyObject *self)
{
    FLEXT_ASSERT(pySamplebuffer_Check(self));
    return (PyObject *)PyString_FromFormat("<Samplebuffer %s>",pySamplebuffer_AS_STRING(self));
}

static long buffer_hash(PyObject *self)
{
    FLEXT_ASSERT(pySamplebuffer_Check(self));
    return (long)(((pySamplebuffer *)self)->buf);
}

static PyObject *buffer_getsymbol(pySamplebuffer* self,void *closure)
{
    if(self->sym)
        return pySymbol_FromSymbol(self->sym);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyGetSetDef buffer_getseters[] = {
    {"symbol",(getter)buffer_getsymbol, NULL, NULL},
    {NULL}  /* Sentinel */
};

static PyObject *buffer_array(PyObject *obj)
{
    PyObject *ret;
#ifdef PY_NUMARRAY
    if(nasupport) {
        pySamplebuffer *self = (pySamplebuffer *)obj;
        if(self->buf) {
            maybelong shape[2];
            shape[0] = self->buf->Frames();
            shape[1] = self->buf->Channels();
            ret = (PyObject *)NA_NewAllFromBuffer(2,shape,numtype,(PyObject *)self,0,sizeof(t_sample *),NA_ByteOrder(),1,1);
        }
        else
            Py_INCREF(ret = Py_None);
    }
    else {
        PyErr_Format(PyExc_RuntimeError,"No numarray support");
        ret = NULL;
    }
#else
    Py_INCREF(ret = Py_None);
#endif
    return ret;
}

static PyObject *buffer_dirty(PyObject *obj)
{
    ((pySamplebuffer *)obj)->dirty = true;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef buffer_methods[] = {
    {"array", (PyCFunction)buffer_array,METH_NOARGS,"Return a numarray object"},
    {"dirty", (PyCFunction)buffer_dirty,METH_NOARGS,"Mark buffer as dirty"},
    {NULL}  /* Sentinel */
};



// support the buffer protocol
static int buffer_readbuffer(PyObject *obj, int segment, void **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static int buffer_writebuffer(PyObject *obj, int segment, void **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static int buffer_segcount(PyObject *obj, int *lenp)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    if(lenp) lenp[0] = b->Channels()*b->Frames()*sizeof(t_sample);
    return 1;
}

static int buffer_charbuffer(PyObject *obj, int segment, const char **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = (char *)b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static PyBufferProcs bufferprocs = {
    buffer_readbuffer,
    buffer_writebuffer,
    buffer_segcount,
    buffer_charbuffer
};

PyTypeObject pySamplebuffer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Buffer",              /*tp_name*/
    sizeof(pySamplebuffer),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    buffer_dealloc,            /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,            /*tp_compare*/
    buffer_repr,               /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    buffer_hash,               /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    &bufferprocs,             /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT /*| Py_TPFLAGS_BASETYPE*/,   /*tp_flags*/
    "Samplebuffer objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0 /*buffer_richcompare*/,	       /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    buffer_methods,                          /* tp_methods */
    0,            /* tp_members */
    buffer_getseters,          /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    buffer_init,            /* tp_init */
    0,                         /* tp_alloc */
    buffer_new,                 /* tp_new */
};

void initsamplebuffer()
{
#ifdef PY_NUMARRAY
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

    numtype = sizeof(t_sample) == 4?tFloat32:tFloat64;
#endif

    if(PyType_Ready(&pySamplebuffer_Type) < 0)
        FLEXT_ASSERT(false);
    else
        Py_INCREF(&pySamplebuffer_Type);
}

