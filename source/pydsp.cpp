/* 

py/pyext - python script object for PD and Max/MSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pyext.h"

class pydsp
    : public pyext
{
	FLEXT_HEADER(pydsp,pyext)
public:
    pydsp(int argc,const t_atom *argv);

protected:
    virtual bool CbDsp();
    virtual void CbSignal();

    virtual bool DoInit();
    virtual void DoExit();

    virtual PyObject *GetSig(int ix,bool in);

    void NewBuffers(bool update = false);
    void FreeBuffers();

    PyObject *dspfun,*sigfun;
    PyObject **buffers;
};

FLEXT_LIB_DSP_V("pyext~ pyext.~ pyx~ pyx.~",pydsp)

pydsp::pydsp(int argc,const t_atom *argv)
    : pyext(argc,argv,true) 
    , dspfun(NULL),sigfun(NULL)
{}

bool pydsp::DoInit()
{
    if(!pyext::DoInit()) return false;
    
    if(pyobj) 
	{ 
        NewBuffers();

        dspfun = PyObject_GetAttrString(pyobj,"_dsp"); // get ref
	    if(dspfun && !PyMethod_Check(dspfun)) {
            Py_DECREF(dspfun);
		    dspfun = NULL;
	    }
        sigfun = PyObject_GetAttrString(pyobj,"_signal"); // get ref
	    if(sigfun && !PyMethod_Check(sigfun)) {
            Py_DECREF(sigfun);
		    sigfun = NULL;
	    }
	}
    return true;
}

void pydsp::DoExit()
{
    if(dspfun) { Py_DECREF(dspfun); dspfun = NULL; }
    if(sigfun) { Py_DECREF(sigfun); sigfun = NULL; }

    FreeBuffers();
}

PyObject *NAFromBuffer(PyObject *buf,int c,int n);

void pydsp::NewBuffers(bool update)
{
    int i,n = Blocksize();
    const int ins = CntInSig(),outs = CntOutSig();
    t_sample *const *insigs = InSig();
    t_sample *const *outsigs = OutSig();

    if(!buffers) {
        int cnt = ins+outs;
        if(cnt) {
            buffers = new PyObject *[cnt];
            memset(buffers,0,cnt*sizeof(*buffers));
        }
    }

    for(i = 0; i < ins; ++i) {
        if(update) Py_XDECREF(buffers[i]);
        PyObject *b = PyBuffer_FromReadWriteMemory(insigs[i],n*sizeof(t_sample));
        buffers[i] = NAFromBuffer(b,1,n);
        Py_DECREF(b);
    }
    for(i = 0; i < outs; ++i) {
        if(update) Py_XDECREF(buffers[ins+i]);
        if(i < ins && outsigs[i] == insigs[i]) {
            // same vectors - share the objects!
            buffers[ins+i] = buffers[i];
            Py_XINCREF(buffers[i]);
        }
        else {
            PyObject *b = PyBuffer_FromReadWriteMemory(outsigs[i],n*sizeof(t_sample));
            buffers[ins+i] = NAFromBuffer(b,1,n);
            Py_DECREF(b);
        }
    }
}

void pydsp::FreeBuffers()
{
    if(buffers) {
        int cnt = CntInSig()+CntOutSig();
        for(int i = 0; i < cnt; ++i) Py_XDECREF(buffers[i]);
        delete[] buffers;
        buffers = NULL;
    }
}

bool pydsp::CbDsp()
{
    if(CntInSig() || CntOutSig())
    {
        NewBuffers(true);

        if(dspfun) {
        	PyThreadState *state = PyLock();
//            Py_INCREF(emptytuple);
            PyObject *ret = PyObject_Call(dspfun,emptytuple,NULL);
//            Py_DECREF(emptytuple);
            if(ret)
                Py_DECREF(ret);
            else {
#ifdef FLEXT_DEBUG
                PyErr_Print();
#else
                PyErr_Clear();
#endif   
            }
            PyUnlock(state);
        }
        return true;
    }
    else
        // switch on dsp only if there are signal inlets or outlets
        return false;
}

void pydsp::CbSignal()
{
    if(sigfun) {
      	PyThreadState *state = PyLock();
//        Py_INCREF(emptytuple);
        PyObject *ret = PyObject_Call(sigfun,emptytuple,NULL);
//        Py_DECREF(emptytuple);

        if(ret) 
            Py_DECREF(ret);
        else {
#ifdef FLEXT_DEBUG
            PyErr_Print();
#else
            PyErr_Clear();
#endif   
        }
        PyUnlock(state);
    }
    else
        flext_dsp::CbSignal();
}

PyObject *pydsp::GetSig(int ix,bool in) 
{
    PyObject *r = buffers[in?ix:CntInSig()+ix];
    Py_XINCREF(r);
    return r;
}

