/*
py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2019 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "pybase.h"

#undef PY_ARRAYS


#if defined(PY_NUMERIC) || defined(PY_NUMPY) || defined(PY_NUMARRAY)
    #define PY_ARRAYS 1
#endif

#ifdef PY_ARRAYS

#ifdef PY_NUMARRAY
#   ifdef PY_USE_FRAMEWORK
#       include <Python/numarray/libnumarray.h>
#   else
#       include <numarray/libnumarray.h>
#   endif

static NumarrayType numtype = tAny;
inline bool arrsupport() { return numtype != tAny; }

#else
#   if defined(PY_NUMPY)
#       include <numpy/arrayobject.h>
#   else
#       ifdef PY_USE_FRAMEWORK
#           include <Python/numarray/arrayobject.h>
#       else
#           include <numarray/arrayobject.h>
#       endif
#   endif

    static PyArray_TYPES numtype = PyArray_NOTYPE;
    inline bool arrsupport() { return numtype != PyArray_NOTYPE; }
#endif
#endif


PyObject *pybase::py_arraysupport(PyObject *self,PyObject *args)
{
    PyObject *ret;
#ifdef PY_ARRAYS
    ret = Py_True;
#else
    ret = Py_False;
#endif
    Py_INCREF(ret);
    return ret;
}


// PD defines a T_OBJECT symbol
#undef T_OBJECT

#ifdef PY_USE_FRAMEWORK
#include "Python/structmember.h"
#else
#include "structmember.h"
#endif

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
        if(self->dirty) 
            self->buf->Dirty(true);
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
#if PY_MAJOR_VERSION < 3
    else if(PyString_Check(arg))
        self->sym = flext::MakeSymbol(PyString_AS_STRING(arg));
#else
    else if(PyUnicode_Check(arg))
        self->sym = flext::MakeSymbol(PyUnicode_AsUTF8(arg));
#endif
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
    return (PyObject *)
#if PY_MAJOR_VERSION < 3
        PyString_FromFormat
#else
        PyUnicode_FromFormat
#endif
            ("<Samplebuffer %s>", pySamplebuffer_AS_STRING(self));
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
    {const_cast<char *>("symbol"),(getter)buffer_getsymbol, NULL, NULL},
    {NULL}  /* Sentinel */
};

static PyObject *buffer_dirty(PyObject *obj)
{
    ((pySamplebuffer *)obj)->dirty = true;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *buffer_resize(PyObject *obj,PyObject *args,PyObject *kwds)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(obj);
    flext::buffer *b = self->buf;
    if(b) {
        int frames,keep = 1,zero = 1;
        static char const *kwlist[] = {"frames", "keep", "zero", NULL};
        if(!PyArg_ParseTupleAndKeywords(args, kwds, "i|ii", (char **)kwlist, &frames, &keep, &zero)) 
            return NULL; 

        b->Frames(frames,keep != 0,zero != 0);

        Py_INCREF(obj);
        return obj;
    }
    else {
        PyErr_SetString(PyExc_RuntimeError,"Invalid buffer");
        return NULL;
    }
}

static PyMethodDef buffer_methods[] = {
    {"dirty", (PyCFunction)buffer_dirty,METH_NOARGS,"Mark buffer as dirty"},
    {"resize", (PyCFunction)buffer_resize,METH_VARARGS|METH_KEYWORDS,"Resize buffer"},
    {NULL}  /* Sentinel */
};

// support the buffer protocol

static Py_ssize_t buffer_readbuffer(PyObject *obj, Py_ssize_t segment, void **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static Py_ssize_t buffer_writebuffer(PyObject *obj, Py_ssize_t segment, void **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static Py_ssize_t buffer_segcount(PyObject *obj, Py_ssize_t *lenp)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    if(lenp) lenp[0] = b->Channels()*b->Frames()*sizeof(t_sample);
    return 1;
}

static Py_ssize_t buffer_charbuffer(PyObject *obj, Py_ssize_t segment,
#if PY_VERSION_HEX < 0x02050000
    const
#endif
    char **ptrptr)
{
    flext::buffer *b = ((pySamplebuffer *)obj)->buf;
    ptrptr[0] = (char *)b->Data();
    return b->Channels()*b->Frames()*sizeof(t_sample);
}

static int buffer_getbuffer(PyObject *obj, Py_buffer *view, int flags) {
    if(flags & PyBUF_INDIRECT) {
        PyErr_SetString(PyExc_BufferError, "PyBUF_INDIRECT not supported");
        view->obj = NULL;
        return -1;
    }

    if(flags & PyBUF_STRIDES) {
        PyErr_SetString(PyExc_BufferError, "PyBUF_STRIDES not supported");
        view->obj = NULL;
        return -1;
    }

    if(flags & PyBUF_ND) {
        PyErr_SetString(PyExc_BufferError, "PyBUF_ND not supported");
        view->obj = NULL;
        return -1;
    }

    if(flags & PyBUF_F_CONTIGUOUS) {
        PyErr_SetString(PyExc_BufferError, "PyBUF_F_CONTIGUOUS not supported");
        view->obj = NULL;
        return -1;
    }

    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(obj);
    flext::buffer *b = self->buf;
    
    view->buf = (void *)b->Data();
    Py_INCREF(obj);
    view->obj = obj;
    view->len = b->Channels()*b->Frames()*sizeof(t_sample);
    view->readonly = false;
    view->itemsize = 1;
    view->format = NULL;
    view->ndim = 1;
    view->shape = NULL;
    view->strides = NULL;
    view->suboffsets = NULL;
    view->internal = NULL;

    if(flags & PyBUF_FORMAT) {
        view->format = "B";
    }

    return 0;
}

static void buffer_releasebuffer(PyObject *obj, Py_buffer *view) {
    // nothing to do here
}

static PyBufferProcs buffer_as_buffer = {
#if PY_MAJOR_VERSION < 3
    buffer_readbuffer,
    buffer_writebuffer,
    buffer_segcount,
    buffer_charbuffer
#else
    buffer_getbuffer,
    buffer_releasebuffer
#endif
};

static Py_ssize_t buffer_length(PyObject *s)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    return self->buf?self->buf->Frames():0;
}

static PyObject *buffer_item(PyObject *s,Py_ssize_t i)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *ret;
    if(self->buf) {
        if (i < 0 || i >= self->buf->Frames()) {
            PyErr_SetString(PyExc_IndexError,"Index out of range");
            ret = NULL;
        }
        else {
            if(self->buf->Channels() == 1)
                ret = PyFloat_FromDouble(self->buf->Data()[i]);
            else {
                PyErr_SetString(PyExc_NotImplementedError,"Multiple channels not implemented yet");
                ret = NULL;
            }
        }
    }
    else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    return ret;
}

#ifndef PY_NUMPY
typedef int npy_intp;
#endif

PyObject *arrayfrombuffer(PyObject *buf,int c,int n)
{
#ifdef PY_ARRAYS
    if(arrsupport()) {
        PyObject *arr;
        npy_intp shape[2] = {n,c};
#ifdef PY_NUMARRAY
        arr = (PyObject *)NA_NewAllFromBuffer(c == 1?1:2,shape,numtype,buf,0,0,NA_ByteOrder(),1,1);
#else
        Py_buffer view;
        int err = PyObject_GetBuffer(buf, &view, PyBUF_SIMPLE | PyBUF_WRITABLE);
        if(!err) {
            FLEXT_ASSERT(view->len <= n*c*sizeof(t_sample));
//            Py_INCREF(buf); // ATTENTION... this won't be released any more!!
#   ifdef PY_NUMPY
            arr = PyArray_NewFromDescr(&PyArray_Type,PyArray_DescrNewFromType(numtype),c == 1?1:2,shape,0,(char *)view.buf,NPY_WRITEABLE|NPY_C_CONTIGUOUS,NULL);
#   else
            arr = PyArray_FromDimsAndData(c == 1?1:2,shape,numtype,(char *)view.buf);
#   endif
        }
        else {
            // exception string is already set
            arr = NULL;
        }
#endif
        return arr;
    }
    else
#endif
    return NULL;
}

static PyObject *buffer_slice(PyObject *s,Py_ssize_t ilow = 0,Py_ssize_t ihigh = 1<<(sizeof(int)*8-2))
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *ret;
#ifdef PY_ARRAYS
    if(arrsupport()) {
        if(self->buf) {
            const int n = self->buf->Frames();
            const int c = self->buf->Channels();
            if(ilow < 0) ilow += n;
            if(ilow >= n) ilow = n-1;
            if(ihigh < 0) ihigh += n;
            if(ihigh > n) ihigh = n;

            PyObject *nobj = arrayfrombuffer((PyObject *)self,c,n);
            if(ilow != 0 || ihigh != n) {
                ret = PySequence_GetSlice(nobj,ilow,ihigh);
                Py_DECREF(nobj);
            }
            else
                ret = nobj;
        }
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    else 
#endif
    {
        PyErr_SetString(PyExc_RuntimeError,"No array support");
        ret = NULL;
    }
    return ret;
}

static int buffer_ass_item(PyObject *s,Py_ssize_t i,PyObject *v)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    int ret;
    if(self->buf) {
        if (i < 0 || i >= self->buf->Frames()) {
            PyErr_Format(PyExc_IndexError,"Index out of range");
            ret = -1;
        }
        else {
            if(self->buf->Channels() == 1) {
                self->buf->Data()[i] = (t_sample)PyFloat_AsDouble(v);
                if(PyErr_Occurred()) {
                    // cast to double failed
                    PyErr_SetString(PyExc_TypeError,"Value must be a array");
                    ret = -1;
                }
                else {
                    self->dirty = true;
                    ret = 0;
                }
            }
            else {
                PyErr_SetString(PyExc_NotImplementedError,"Multiple channels not implemented yet");
                ret = -1;
            }
        }
    }
    else
        ret = -1;
    return ret;
}

static int buffer_ass_slice(PyObject *s,Py_ssize_t ilow,Py_ssize_t ihigh,PyObject *value)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    int ret;
#ifdef PY_ARRAYS
    if(arrsupport()) {
        if(!value) {
            PyErr_SetString(PyExc_TypeError,"Object doesn't support item deletion");
            ret = -1;
        }
        else if(self->buf) {
            const int n = self->buf->Frames();
//            const int c = self->buf->Channels();
            if(ilow < 0) ilow += n;
            if(ilow >= n) ilow = n-1;
            if(ihigh < 0) ihigh += n;
            if(ihigh > n) ihigh = n;

#ifdef PY_NUMARRAY
            PyArrayObject *out = NA_InputArray(value,numtype,NUM_C_ARRAY);
            const t_sample *src = (t_sample *)NA_OFFSETDATA(out);
#else
            PyArrayObject *out = (PyArrayObject *)PyArray_ContiguousFromObject(value,numtype,1,2);
            const t_sample *src = (t_sample *)out->data;
#endif
            if(!out) {
                // exception already set
                ret = -1;
            }
            else if(out->nd != 1) {
                PyErr_SetString(PyExc_NotImplementedError,"Multiple dimensions not supported yet");
                ret = -1;
            }
            else {
                int dlen = ihigh-ilow;
                int slen = out->dimensions[0];
                int cnt = slen < dlen?slen:dlen;
                flext::buffer::Element *dst = self->buf->Data()+ilow;
                for(int i = 0; i < cnt; ++i) dst[i] = src[i];
                
                self->dirty = true;
                ret = 0;
            }

            Py_XDECREF(out);
        }
        else {
            PyErr_SetString(PyExc_ValueError,"Buffer is not assigned");
            ret = -1;
        }
    }
    else 
#endif
    {
        PyErr_SetString(PyExc_RuntimeError,"No array support");
        ret = -1;
    }
    return ret;
}

static PyObject *buffer_concat(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PySequence_Concat(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_repeat(PyObject *s,Py_ssize_t rep)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PySequence_Repeat(nobj,rep);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}


static PySequenceMethods buffer_as_seq = {
    buffer_length,          /* lenfunc sq_length              __len__ */
    buffer_concat,          /* binaryfunc sq_concat           __add__ */
    buffer_repeat,          /* ssizeargfunc sq_repeat         __mul__ */
    buffer_item,            /* ssizeargfunc sq_item;          __getitem__ */
    NULL,                   /* ssizeobjargproc sq_ass_item    __setitem__ */
    NULL,                   /* objobjproc sq_contains         __contains__ */
    NULL,                   /* binaryfunc sq_inplace_concat   __iadd__ */
    NULL                    /* ssizeargfunc sq_inplace_repeat __imul */
};

static PyObject *buffer_iter(PyObject *s)
{
//    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *it = PyObject_GetIter(nobj);
        Py_DECREF(nobj);
        return it;
    }
    else
        return NULL;
}


static PyObject *buffer_add(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Add(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_subtract(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Subtract(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_multiply(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Multiply(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

#if PY_MAJOR_VERSION < 3
static PyObject *buffer_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Divide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}
#endif

static PyObject *buffer_true_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_TrueDivide(nobj, op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_floor_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_FloorDivide(nobj, op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_remainder(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Remainder(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_divmod(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Divmod(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_power(PyObject *s,PyObject *op1,PyObject *op2)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Power(nobj,op1,op2);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_negative(PyObject *s)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Negative(nobj);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_pos(PyObject *s)
{
//    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Positive(nobj);
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_absolute(PyObject *s)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_Absolute(nobj);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static int buffer_coerce(PyObject **pm, PyObject **pw) 
{
    if(pySamplebuffer_Check(*pw)) {
        Py_INCREF(*pm);
        Py_INCREF(*pw);
        return 0;
    }
    else
        return 1;
}
    
static PyObject *buffer_inplace_add(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceAdd(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_subtract(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceSubtract(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_multiply(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceMultiply(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

#if PY_MAJOR_VERSION < 3
static PyObject *buffer_inplace_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceDivide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}
#endif

static PyObject *buffer_inplace_true_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceTrueDivide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_floor_divide(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceFloorDivide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_remainder(PyObject *s,PyObject *op)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceRemainder(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_power(PyObject *s,PyObject *op1,PyObject *op2)
{
    pySamplebuffer *self = reinterpret_cast<pySamplebuffer *>(s);
    PyObject *nobj = buffer_slice(s);
    if(nobj) {
        PyObject *ret = PyNumber_InPlacePower(nobj,op1,op2);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}



static PyNumberMethods buffer_as_number = {
    (binaryfunc)buffer_add, // binaryfunc nb_add
    (binaryfunc)buffer_subtract, // binaryfunc nb_subtract
    (binaryfunc)buffer_multiply, // binaryfunc nb_multiply
#if PY_MAJOR_VERSION < 3
    (binaryfunc)buffer_divide, // binaryfunc nb_divide
#endif
    (binaryfunc)buffer_remainder, // nb_binaryfunc remainder
    (binaryfunc)buffer_divmod, // binaryfunc nb_divmod
    (ternaryfunc)buffer_power, // ternaryfunc nb_power
    (unaryfunc)buffer_negative, // unaryfunc nb_negative
    (unaryfunc)buffer_pos, // unaryfunc nb_pos
    (unaryfunc)buffer_absolute, // unaryfunc np_absolute
    0, //(inquiry)buffer_nonzero, // inquiry nb_nonzero
    0, // unaryfunc nb_invert
    0, // binaryfunc nb_lshift
    0, // binaryfunc nb_rshift
    0, // binaryfunc nb_and
    0, // binaryfunc nb_xor
    0, // binaryfunc nb_or
#if PY_MAJOR_VERSION < 3
    (coercion)buffer_coerce, // coercion nb_coerce
#endif
    0, // unaryfunc nb_int
#if PY_MAJOR_VERSION < 3
    0, // unaryfunc nb_long
    0, // unaryfunc nb_float
    0, // unaryfunc nb_oct
    0, // unaryfunc nb_hex
#else
    0, // void *nb_reserved
    0, // unaryfunc nb_float
#endif
    (binaryfunc)buffer_inplace_add, // binaryfunc nb_inplace_add
    (binaryfunc)buffer_inplace_subtract, // binaryfunc nb_inplace_subtract
    (binaryfunc)buffer_inplace_multiply, // binaryfunc nb_inplace_multiply
#if PY_MAJOR_VERSION < 3
    (binaryfunc)buffer_inplace_divide, // binaryfunc nb_inplace_divide
#endif
    (binaryfunc)buffer_inplace_remainder, // binaryfunc nb_inplace_remainder
    (ternaryfunc)buffer_inplace_power, // ternaryfunc nb_inplace_power
    0, // binaryfunc nb_inplace_lshift
    0, // binaryfunc nb_inplace_rshift
    0, // binaryfunc nb_inplace_and
    0, // binaryfunc nb_inplace_xor
    0, // binaryfunc nb_inplace_or
    (binaryfunc)buffer_floor_divide, // binaryfunc nb_floor_divide
    (binaryfunc)buffer_true_divide, // binaryfunc nb_true_divide
    (binaryfunc)buffer_inplace_floor_divide, // binaryfunc nb_inplace_floor_divide
    (binaryfunc)buffer_inplace_true_divide, // binaryfunc nb_inplace_true_divide
};

PyTypeObject pySamplebuffer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Buffer",              /*tp_name*/
    sizeof(pySamplebuffer),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    buffer_dealloc,            /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,            /*tp_compare*/
    buffer_repr,               /*tp_repr*/
    &buffer_as_number,                         /*tp_as_number*/
    &buffer_as_seq,                 /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    buffer_hash,               /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    &buffer_as_buffer,             /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT /*| Py_TPFLAGS_BASETYPE*/   /*tp_flags*/
#if PY_MAJOR_VERSION < 3
    | Py_TPFLAGS_HAVE_NEWBUFFER
#endif
    ,
    "Samplebuffer objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0 /*buffer_richcompare*/,          /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    buffer_iter,                       /* tp_iter */
    0,                     /* tp_iternext */
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

// Must have this as a function because the import_array macro in numpy version 1.01 strangely has a return statement included.
// Furthermore the import error printout from this macro is ugly, but we accept that for now, waiting for later numpy updates to fix all of this.
// The situation is further complicated by Python 3, where numpy's import_array returns NULL...
#ifdef PY_ARRAYS
#ifdef PY_NUMARRAY
#define IMPORT_ARRAY_RET_TYPE void
#elif PY_MAJOR_VERSION < 3
#define IMPORT_ARRAY_RET_TYPE void
#else
#define IMPORT_ARRAY_RET_TYPE void *
#endif
static IMPORT_ARRAY_RET_TYPE __import_array__()
{
#ifdef PY_NUMARRAY
    import_libnumarray();
#else
    import_array();
#endif
}
#endif

void initsamplebuffer()
{
#ifdef PY_ARRAYS
    __import_array__();
    if(PyErr_Occurred())
        // catch import error
        PyErr_Clear();
    else {
        // numarray support ok
#ifdef PY_NUMARRAY
        numtype = sizeof(t_sample) == 4?tFloat32:tFloat64;
#else
        numtype = sizeof(t_sample) == 4?PyArray_FLOAT:PyArray_DOUBLE;
#endif
        post("");
        post("Python array support enabled");
    }
#endif

    if(PyType_Ready(&pySamplebuffer_Type) < 0)
        FLEXT_ASSERT(false);
    else
        Py_INCREF(&pySamplebuffer_Type);
}
