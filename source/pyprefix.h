/*
py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2015 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __PYPREFIX_H
#define __PYPREFIX_H

#define FLEXT_ATTRIBUTES 1
#include <flext.h>

// hack: must include math.h before Python.h (at least on OSX)
// otherwise some functions don't get defined
#include <cmath>

#ifdef PY_USE_FRAMEWORK
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 501)
#error You need at least flext version 0.5.1
#endif

#if FLEXT_OS == FLEXT_LINUX || FLEXT_OS == FLEXT_IRIX
#include <unistd.h>
#endif

#if FLEXT_SYS == FLEXT_SYS_PD && (!defined (PD_MINOR_VERSION) || ( PD_MAJOR_VERSION == 0 && PD_MINOR_VERSION < 37))
#error PD version >= 0.37 required, please upgrade! 
#endif

#include <flcontainers.h>
#include <string>

#if FLEXT_SYS == FLEXT_SYS_PD && defined(PY_USE_INOFFICIAL)
extern "C" {
#include <s_stuff.h>
}
#endif

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

// these are copied from the Python 3.8.2 source because doing the right thing
// with the pre-3.6.1 slice functions seems difficult...

#if PY_MAJOR_VERSION < 3
/* Assert a build-time dependency, as an expression.
   Your compile will fail if the condition isn't true, or can't be evaluated
   by the compiler. This can be used in an expression: its value is 0.
   Example:
   #define foo_to_char(foo)  \
       ((char *)(foo)        \
        + Py_BUILD_ASSERT_EXPR(offsetof(struct foo, string) == 0))
   Written by Rusty Russell, public domain, http://ccodearchive.net/ */
#define Py_BUILD_ASSERT_EXPR(cond) \
    (sizeof(char [1 - 2*!(cond)]) - 1)

#define Py_BUILD_ASSERT(cond)  do {         \
        (void)Py_BUILD_ASSERT_EXPR(cond);   \
    } while(0)
#endif

#if PY_VERSION_HEX < 0x03060100
#endif

#endif
