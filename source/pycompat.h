#ifndef __PYCOMPAT_H
#define __PYCOMPAT_H

#ifdef PY_USE_FRAMEWORK
#include <Python/Python.h>
#else
#include <Python.h>
#endif

// copied from Python 3.8.2 for compatibility with pre-3.6.1 versions because
// doing the right thing with the older slice functions seems difficult...

#if PY_VERSION_HEX < 0x03060100
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

int
PySlice_Unpack(PyObject *_r,
               Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t *step);

Py_ssize_t
PySlice_AdjustIndices(Py_ssize_t length,
                      Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t step);
#endif

#endif
