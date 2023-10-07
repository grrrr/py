/*
py/pyext - python external object for PD and Max/MSP

Copyright (c)2002-2019 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "pyext.h"


PyMethodDef pyext::meth_tbl[] = 
{
/*
    {"__init__", pyext::pyext__init__, METH_VARARGS, "Constructor"},
    {"__del__", pyext::pyext__del__, METH_VARARGS, "Destructor"},
*/
    {"__str__", pyext::pyext__str__, METH_VARARGS, "stringify"},
    {"_outlet", pyext::pyext_outlet, METH_VARARGS,"Send message to outlet"},
#if FLEXT_SYS == FLEXT_SYS_PD
    {"_tocanvas", pyext::pyext_tocanvas, METH_VARARGS,"Send message to canvas" },
#endif

    { "_bind", pyext::pyext_bind, METH_VARARGS,"Bind function to a receiving symbol" },
    { "_unbind", pyext::pyext_unbind, METH_VARARGS,"Unbind function from a receiving symbol" },
#ifdef FLEXT_THREADS
    { "_detach", pyext::pyext_detach, METH_VARARGS,"Set detach flag for called methods" },
    { "_stop", pyext::pyext_stop, METH_VARARGS,"Stop running threads" },
#endif
    { "_invec", pyext::pyext_invec, METH_VARARGS,"Get input vector" },
    { "_outvec", pyext::pyext_outvec, METH_VARARGS,"Get output vector" },
    { "__setattr__", pyext::pyext_setattr, METH_VARARGS,"Set class attribute" },
    { "__getattr__", pyext::pyext_getattr, METH_VARARGS,"Get class attribute" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

const char *pyext::pyext_doc =
    "py/pyext - python external object for PD and Max/MSP, (C)2002-2008 Thomas Grill\n"
    "\n"
    "This is the pyext base class. Available methods:\n"
    "_outlet(self,ix,args...): Send a message to an indexed outlet\n"
#if FLEXT_SYS == FLEXT_SYS_PD
    "_tocanvas(self,args...): Send a message to the parent canvas\n"
#endif
    "_bind(self,name,func): Bind a python function to a symbol\n"
    "_unbind(self,name,func): Unbind a python function from a symbol\n"
    "_isthreaded: Query whether threading is enabled\n"
#ifdef FLEXT_THREADS
    "_detach(self,int): Define whether a called Python method has its own thread\n"
    "_stop(self): Stop running threads\n"
    "_shouldexit: Query whether threads should terminate\n"
#endif
;

PyObject* pyext::pyext__str__(PyObject *self, PyObject *args)
{   
    return
#if PY_MAJOR_VERSION < 3
        PyString_FromFormat
#else
        PyUnicode_FromFormat
#endif
            ("<pyext object %p>", self);
}

PyObject* pyext::pyext_setattr(PyObject *self, PyObject *args)
{
    PyObject *name,*val;
    if(!PyArg_ParseTuple(args, "OO:pyext_setattr",&name,&val))
    {
        // handle error
        ERRINTERNAL();
        return NULL;
    }
    
    
    bool handled = false;

/*
    if(PyString_Check(name)) {
        char* sname = PyString_AsString(name);
        if (sname) {
//          post("pyext::setattr %s",sname);
        }
    }
*/
    if(!handled) {
        if(PyObject_GenericSetAttr(self, name, val) < 0) {
            ERRINTERNAL();
            return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* pyext::pyext_getattr(PyObject *self, PyObject *args)
{
    PyObject *name, *ret = NULL;
    if(!PyArg_ParseTuple(args, "O:pyext_getattr", &name))
    {
        // handle error
        ERRINTERNAL();
        return NULL;
    }

#if PY_MAJOR_VERSION < 3
    if(PyString_Check(name))
#else
    if(PyUnicode_Check(name))
#endif
    {
#if PY_MAJOR_VERSION < 3
        const char *sname = PyString_AS_STRING(name);
#else
        const char *sname = PyUnicode_AsUTF8(name);
#endif
        if(sname) {
#ifdef FLEXT_THREADS
            if(!strcmp(sname,"_shouldexit")) {
                pyext *ext = GetThis(self); 
                if(ext)
                    ret = PyLong_FromLong(ext->shouldexit?1:0);
                else {
                    // return true for _shouldexit if association has been removed
                    Py_INCREF(Py_True);
                    ret = Py_True;
                }
            }
            else 
#endif
            if(!strcmp(sname,"_isthreaded")) {
            #ifdef FLEXT_THREADS
                Py_INCREF(Py_True);
                ret = Py_True;
            #else
                Py_INCREF(Py_False);
                ret = Py_False;
            #endif
            }
            else if(!strcmp(sname, "_canvas_dir")) {
                pyext *ext = GetThis(self); 
                char dir[1024];
                ext->GetCanvasDir(dir, sizeof(dir));
#if PY_MAJOR_VERSION < 3
                ret = PyString_InternFromString(dir);
#else
                ret = PyUnicode_InternFromString(dir);
#endif
            }
        }
    }

    if(!ret) { 
#if PY_VERSION_HEX >= 0x02020000
        ret = PyObject_GenericGetAttr(self,name); // new reference (?)
#else
        if(PyInstance_Check(self))
            // borrowed reference
            ret = PyDict_GetItem(((PyInstanceObject *)self)->in_dict,name); 
#endif
    }
    return ret;
}

//! Send message to outlet
PyObject *pyext::pyext_outlet(PyObject *self, PyObject *args)
{
    bool ok = false;

    // should always be a tuple!
    FLEXT_ASSERT(PyTuple_Check(args));

    int sz = PyTuple_GET_SIZE(args);

    // borrowed references!
    PyObject *outl;

    if(
        sz >= 1 &&
        (outl = PyTuple_GET_ITEM(args,0)) != NULL &&
#if PY_MAJOR_VERSION < 3
            PyInt_Check(outl)
#else
            PyLong_Check(outl)
#endif
    ) {
        pyext *ext = GetThis(self);
        if(!ext) {    
            PyErr_SetString(PyExc_RuntimeError,"pyext - _outlet: instance not associated with pd object");
            return NULL;
        }

        PyObject *val;
#if 0
        if(sz == 2) {
            val = PyTuple_GET_ITEM(args,1); // borrow reference
            Py_INCREF(val);
            tp = PySequence_Check(val);
        }
        else
            tp = false;

        if(!tp)
            val = PySequence_GetSlice(args,1,sz);  // new ref
#else
        if(sz == 2) {
            val = PyTuple_GET_ITEM(args,1); // borrow reference
            Py_INCREF(val);
        }
        else
            val = PyTuple_GetSlice(args,1,sz);  // new ref
#endif

        int o;
#if PY_MAJOR_VERSION < 3
        o = PyInt_AS_LONG(outl);
#else
        o = PyLong_AS_LONG(outl);
#endif
        if(o >= 1 && o <= ext->Outlets()) {
            // offset outlet by signal outlets
            o += ext->sigoutlets;

            if(ext->OutObject(ext,o-1,val))
                ok = true;
            else 
                PyErr_SetString(PyExc_ValueError,"pyext - _outlet: invalid arguments");
        }
        else
            PyErr_SetString(PyExc_ValueError,"pyext - _outlet: index out of range");

        Py_DECREF(val);
    }
    else
        PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _outlet(self,outlet,args...)");

    if(!ok) return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}



#ifdef FLEXT_THREADS
//! Detach threads
PyObject *pyext::pyext_detach(PyObject *self, PyObject *args)
{
    int val;
    if(
        !PyArg_ParseTuple(args, "i:pyext_detach",&val)
    ) {
        // handle error
        PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _detach(self,[0/1/2])");
        return NULL;
    }
    else if(val < 0 || val > 2) {
        PyErr_SetString(PyExc_ValueError,"pyext - _detach must be in the range 0..2");
        return NULL;
    }
    else {
        pyext *ext = GetThis(self);
        if(!ext) {    
            PyErr_SetString(PyExc_RuntimeError,"pyext - _detach: instance not associated with pd object");
            return NULL;
        }

        ext->detach = val;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

//! Stop running threads
PyObject *pyext::pyext_stop(PyObject *self, PyObject *args)
{
    int val = -1;
    if(
        !PyArg_ParseTuple(args, "|i:pyext_stop",&val)
    ) {
        // handle error
        PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _stop(self,{wait time})");
        return NULL;
    }
    else if(val < 0) {
        PyErr_SetString(PyExc_ValueError,"pyext - _stop time must be >= 0");
        return NULL;
    }
    else {
        pyext *ext = GetThis(self);
        if(!ext) {    
            PyErr_SetString(PyExc_RuntimeError,"pyext - _stop: instance not associated with pd object");
            return NULL;
        }

        int cnt;
        t_atom at;
        if(val >= 0) cnt = 1,flext::SetInt(at,val);
        else cnt = 0;
        ext->m_stop(cnt,&at);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#endif


#if FLEXT_SYS == FLEXT_SYS_PD
//! Send message to canvas
PyObject *pyext::pyext_tocanvas(PyObject *self, PyObject *args)
{
    FLEXT_ASSERT(PyTuple_Check(args));

    int sz = PyTuple_GET_SIZE(args);

    bool ok = false;
    pyext *ext = GetThis(self);
    if(!ext) {
        PyErr_SetString(PyExc_RuntimeError,"pyext - _tocanvas: instance not associated with pd object");
        return NULL;
    }

    PyObject *val;

    bool tp = 
        sz == 2 && 
        PySequence_Check(
            val = PyTuple_GET_ITEM(args,0) // borrowed ref
        );

    if(!tp)
        val = PyTuple_GetSlice(args,0,sz);  // new ref

    flext::AtomListStatic<16> lst;
    const t_symbol *sym = GetPyArgs(lst,val);
    if(sym) {
        t_glist *gl = ext->thisCanvas();
        if(gl) {
            // \TODO find a flext-based non-locking method
            sys_lock();
            pd_forwardmess((t_class **)gl,lst.Count(),lst.Atoms());
            sys_unlock();
        }
#ifdef FLEXT_DEBUG
        else
            post("pyext - no parent canvas?!");
#endif
        ok = true;
    }
    else 
        post("py/pyext - No data to send");

    if(!tp) Py_DECREF(val);

if(!ok) {
    PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _tocanvas(self,args...)");
    return NULL;
 }

Py_INCREF(Py_None);
return Py_None;
}
#endif

PyObject *pyext::pyext_invec(PyObject *self, PyObject *args)
{
    int val = -1;
    if(
        !PyArg_ParseTuple(args, "|i:pyext_invec",&val)
    ) {
        // handle error
        PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _invec(self,inlet)");
        return NULL;
    }
    else if(val < 0) {
        PyErr_SetString(PyExc_ValueError,"pyext - _invec: index out of range");
        return NULL;
    }
    else {
        pyext *ext = GetThis(self);
        if(ext) {
            PyObject *b = ext->GetSig(val,true);
            if(b) return b;
        }
        else {
            PyErr_SetString(PyExc_RuntimeError,"pyext - _invec: instance not associated with pd object");
            return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *pyext::pyext_outvec(PyObject *self, PyObject *args)
{
    int val = -1;
    if(
        !PyArg_ParseTuple(args, "|i:pyext_outvec",&val)
    ) {
        // handle error
        PyErr_SetString(PyExc_SyntaxError,"pyext - Syntax: _outvec(self,inlet)");
        return NULL;
    }
    else if(val < 0) {
        PyErr_SetString(PyExc_ValueError,"pyext - _outvec: index out of range");
        return NULL;
    }
    else {
        pyext *ext = GetThis(self);
        if(ext) {
            PyObject *b = ext->GetSig(val,false);
            if(b) return b;
        }
        else {
            PyErr_SetString(PyExc_RuntimeError,"pyext - _outvec: instance not associated with pd object");
            return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
