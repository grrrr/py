# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""This is an example script for the py/pyext object's buffer support.

PD/Max buffers can be imported to and exported from numarray arrays.
For numarray see http://numeric.scipy.org
It will probably once be replaced by Numeric

- _import(buffer): copy contents from the buffer to a new numarray object
- _export(buffer,numarray): export contents of numarray object to the buffer
"""

import sys

try:
    import pyext
except:
    print "ERROR: This script must be loaded by the PD/Max pyext external"

try:    
    from numarray import *
except:
    print "Failed importing numarray module:",sys.exc_value

def mul(*args):
    c = pyext.Buffer(args[0])
    dst = c.array()
    dst[:] = 0
    a = pyext.Buffer(args[1]).array()
    b = pyext.Buffer(args[2]).array()
    dst += a*b
    c.dirty()   

def add(*args):
    c = pyext.Buffer(args[0])
    dst = c.array()
    dst[:] = 0
    a = pyext.Buffer(args[1]).array()
    b = pyext.Buffer(args[2]).array()
    dst += a+b
    c.dirty()   
