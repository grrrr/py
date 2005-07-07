/* 

py/pyext - python script object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __PYPREFIX_H
#define __PYPREFIX_H

#define FLEXT_ATTRIBUTES 1
#include <flext.h>

#if FLEXT_OS == FLEXT_OS_MAC
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#if FLEXT_OS == FLEXT_LINUX || FLEXT_OS == FLEXT_IRIX
#include <unistd.h>
#endif

#if FLEXT_SYS == FLEXT_SYS_PD && (!defined (PD_MINOR_VERSION) || PD_MINOR_VERSION < 37)
#error PD version >= 0.37 required, please upgrade! 
#endif

#include <flcontainers.h>
#include <string>


#endif
