/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pybase.h"

#undef PY_ARRAYS


#if defined(PY_NUMERIC)
    #define PY_ARRAYS 1
#elif defined(PY_NUMARRAY)
    #define PY_ARRAYS 1
    #define NA
#endif

#ifdef PY_ARRAYS

#ifdef NA
    #if FLEXT_OS == FLEXT_OS_MAC
    #include <Python/numarray/libnumarray.h>
    #else
    #include <numarray/libnumarray.h>
    #endif

static NumarrayType numtype = tAny;
inline bool arrsupport() { return numtype != tAny; }

#else
    #if FLEXT_OS == FLEXT_OS_MAC
    #include <Python/numarray/arrayobject.h>
    #else
    #include <numarray/arrayobject.h>
    #endif

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

#if FLEXT_OS == FLEXT_OS_MAC
#include "Python/bufferobject.h"
#include "Python/structmember.h"
#else
#include "bufferobject.h"
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

static PyObject *buffer_dirty(PyObject *obj)
{
    ((pySamplebuffer *)obj)->dirty = true;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef buffer_methods[] = {
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

static PyBufferProcs buffer_as_buffer = {
    buffer_readbuffer,
    buffer_writebuffer,
    buffer_segcount,
    buffer_charbuffer
};

static int buffer_length(pySamplebuffer *self)
{
    return self->buf?self->buf->Frames():0;
}

static PyObject *buffer_item(pySamplebuffer *self, int i)
{
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

PyObject *arrayfrombuffer(PyObject *buf,int c,int n)
{
#ifdef PY_ARRAYS
    if(arrsupport()) {
        PyObject *arr;
        int shape[2];
        shape[0] = n;
        shape[1] = c;
#ifdef NA
        arr = (PyObject *)NA_NewAllFromBuffer(c == 1?1:2,shape,numtype,buf,0,0,NA_ByteOrder(),1,1);
#else
        void *data;
        int len;
        int err = PyObject_AsWriteBuffer(buf,&data,&len);
        if(!err) {
            FLEXT_ASSERT(len <= n*c*sizeof(t_sample));
            Py_INCREF(buf);
            arr = PyArray_FromDimsAndData(c == 1?1:2,shape,numtype,(char *)data);
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

static PyObject *buffer_slice(pySamplebuffer *self,int ilow = 0,int ihigh = 1<<(sizeof(int)*8-2))
{
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
        PyErr_SetString(PyExc_RuntimeError,"No numarray support");
        ret = NULL;
    }
    return ret;
}

static int buffer_ass_item(pySamplebuffer *self,int i,PyObject *v)
{
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
    		        PyErr_SetString(PyExc_TypeError,"Value must be a numarray");
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

static int buffer_ass_slice(pySamplebuffer *self,int ilow,int ihigh,PyObject *value)
{
    int ret;
#ifdef PY_ARRAYS
    if(arrsupport()) {
        if(!value) {
            PyErr_SetString(PyExc_TypeError,"Object doesn't support item deletion");
            ret = -1;
        }
        else if(self->buf) {
            const int n = self->buf->Frames();
            const int c = self->buf->Channels();
            if(ilow < 0) ilow += n;
            if(ilow >= n) ilow = n-1;
            if(ihigh < 0) ihigh += n;
            if(ihigh > n) ihigh = n;

#ifdef NA
            PyArrayObject *out = NA_InputArray(value,numtype,NUM_C_ARRAY);
#else
            PyArrayObject *out = (PyArrayObject *)PyArray_ContiguousFromObject(value,numtype,0,0);
#endif
            if(!out) {
                PyErr_SetString(PyExc_TypeError,"Assigned object must be a numarray");
                ret = -1;
            }
            else if(out->nd != 1) {
                PyErr_SetString(PyExc_NotImplementedError,"Multiple dimensions not supported yet");
                ret = -1;
            }
            else {
                int dlen = ihigh-ilow;
                int slen = out->dimensions[0];
#ifdef NA
                flext::CopySamples(self->buf->Data()+ilow,(t_sample *)NA_OFFSETDATA(out),slen < dlen?slen:dlen);
#else
                flext::CopySamples(self->buf->Data()+ilow,(t_sample *)out->data,slen < dlen?slen:dlen);
#endif
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
        PyErr_SetString(PyExc_RuntimeError,"No numarray support");
        ret = -1;
    }
    return ret;
}

static PyObject *buffer_concat(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PySequence_Concat(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_repeat(pySamplebuffer *self,int rep)
{
    PyObject *nobj = buffer_slice(self);
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
	(inquiry)buffer_length,			/* inquiry sq_length;             __len__ */
	(binaryfunc)buffer_concat,          /* __add__ */
	(intargfunc)buffer_repeat,          /* __mul__ */
	(intargfunc)buffer_item,			/* intargfunc sq_item;            __getitem__ */
	(intintargfunc)buffer_slice,		 /* intintargfunc sq_slice;        __getslice__ */
	(intobjargproc)buffer_ass_item,		/* intobjargproc sq_ass_item;     __setitem__ */
	(intintobjargproc)buffer_ass_slice,	/* intintobjargproc sq_ass_slice; __setslice__ */
};

static PyObject *buffer_iter(PyObject *obj)
{
    pySamplebuffer *self = (pySamplebuffer *)obj;
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *it = PyObject_GetIter(nobj);
        Py_DECREF(nobj);
        return it;
    }
    else
        return NULL;
}


static PyObject *buffer_add(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Add(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_subtract(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Subtract(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_multiply(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Multiply(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_divide(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Divide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_remainder(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Remainder(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_divmod(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Divmod(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_power(pySamplebuffer *self,PyObject *op1,PyObject *op2)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Power(nobj,op1,op2);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_negative(pySamplebuffer *self)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Negative(nobj);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_pos(pySamplebuffer *self)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Positive(nobj);
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_absolute(pySamplebuffer *self)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_Absolute(nobj);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static int buffer_coerce(pySamplebuffer **pm, PyObject **pw) 
{
    if(pySamplebuffer_Check(*pw)) {
        Py_INCREF(*pm);
        Py_INCREF(*pw);
    	return 0;
    }
    else
        return 1;
}
	
static PyObject *buffer_inplace_add(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceAdd(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_subtract(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceSubtract(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_multiply(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceMultiply(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_divide(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceDivide(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_remainder(pySamplebuffer *self,PyObject *op)
{
    PyObject *nobj = buffer_slice(self);
    if(nobj) {
        PyObject *ret = PyNumber_InPlaceRemainder(nobj,op);
        if(ret == nobj) self->dirty = true;
        Py_DECREF(nobj);
        return ret;
    }
    else
        return NULL;
}

static PyObject *buffer_inplace_power(pySamplebuffer *self,PyObject *op1,PyObject *op2)
{
    PyObject *nobj = buffer_slice(self);
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
	(binaryfunc)buffer_add, /*nb_add*/
	(binaryfunc)buffer_subtract, /*nb_subtract*/
	(binaryfunc)buffer_multiply, /*nb_multiply*/
	(binaryfunc)buffer_divide, /*nb_divide*/
	(binaryfunc)buffer_remainder, /*nb_remainder*/
	(binaryfunc)buffer_divmod, /*nb_divmod*/
	(ternaryfunc)buffer_power, /*nb_power*/
	(unaryfunc)buffer_negative, 
	(unaryfunc)buffer_pos, /*nb_pos*/ 
	(unaryfunc)buffer_absolute, /* (unaryfunc)buffer_abs,  */
	0, //(inquiry)buffer_nonzero, /*nb_nonzero*/
	0,		/*nb_invert*/
	0,		/*nb_lshift*/
	0,		/*nb_rshift*/
	0,		/*nb_and*/
	0,		/*nb_xor*/
	0,		/*nb_or*/
	(coercion)buffer_coerce, /*nb_coerce*/
	0, /*nb_int*/
	0, /*nb_long*/
	0, /*nb_float*/
	0,		/*nb_oct*/
	0,		/*nb_hex*/
	(binaryfunc)buffer_inplace_add,		/* nb_inplace_add */
	(binaryfunc)buffer_inplace_subtract,		/* nb_inplace_subtract */
	(binaryfunc)buffer_inplace_multiply,		/* nb_inplace_multiply */
	(binaryfunc)buffer_inplace_divide,		/* nb_inplace_divide */
	(binaryfunc)buffer_inplace_remainder,		/* nb_inplace_remainder */
	(ternaryfunc)buffer_inplace_power, 		/* nb_inplace_power */
	0,		/* nb_inplace_lshift */
	0,		/* nb_inplace_rshift */
	0,		/* nb_inplace_and */
	0,		/* nb_inplace_xor */
	0,		/* nb_inplace_or */
//	buffer_floor_div, /* nb_floor_divide */
//	buffer_div,	/* nb_true_divide */
//	buffer_inplace_floor_div,		/* nb_inplace_floor_divide */
//	buffer_inplace_div,		/* nb_inplace_true_divide */
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
    &buffer_as_number,                         /*tp_as_number*/
    &buffer_as_seq,                 /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    buffer_hash,               /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    &buffer_as_buffer,             /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT /*| Py_TPFLAGS_BASETYPE*/,   /*tp_flags*/
    "Samplebuffer objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0 /*buffer_richcompare*/,	       /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    buffer_iter,		               /* tp_iter */
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
    PyErr_Clear();

#ifdef PY_ARRAYS
#ifdef NA
    import_libnumarray();
#else
    import_array();
#endif
    if(PyErr_Occurred())
        // catch import error
        PyErr_Clear();
    else {
        // numarray support ok
#ifdef NA
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
