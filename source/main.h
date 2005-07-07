/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __MAIN_H
#define __MAIN_H

#include "pyprefix.h"

#define PY__VERSION "0.2.1pre"


#define PYEXT_MODULE "pyext" // name for module
#define PYEXT_CLASS "_class"  // name for base class

#define REGNAME "_registry"

#define PY_STOP_WAIT 100  // ms
#define PY_STOP_TICK 1  // ms


class FifoEl
    : public Fifo::Cell
{
public:
    void Set(PyObject *f,PyObject *a) { fun = f,args = a; }
    PyObject *fun,*args;
};

typedef PooledFifo<FifoEl> PyFifo;

#endif
