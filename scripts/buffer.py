# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""This is an example script for the py/pyext object's buffer support.

PD/Max buffers can be mapped to numarray arrays.
For numarray see http://numeric.scipy.org
It will probably once be replaced by Numeric(3)
"""

import sys

try:
    import pyext
except:
    print "ERROR: This script must be loaded by the PD/Max py/pyext external"

try:    
    from numarray import *
except:
    print "Failed importing numarray module:",sys.exc_value

def mul(*args):
    # create buffer objects
    # as long as these variables live the underlying buffers are locked
    c = pyext.Buffer(args[0])
    a = pyext.Buffer(args[1])
    b = pyext.Buffer(args[2])

    # slicing causes numarrays (mapped to buffers) to be created
    # note the c[:] - to assign contents you must assign to a slice of the buffer
    c[:] = a[:]*b[:]  

def add(*args):
    c = pyext.Buffer(args[0])
    a = pyext.Buffer(args[1])
    b = pyext.Buffer(args[2])

    # this is also possible, but is probably slower
    # the + converts a into a numarray, the argument b is taken as a sequence
    # depending on the implementation in numarray this may be as fast
    # as above or not
    c[:] = a+b  

def fadein(target):
    a = pyext.Buffer(target)
    # in place operations are ok
    a *= arange(len(a),type=Float32)/len(a)

def neg(target):
    a = pyext.Buffer(target)
    # in place transformation (see numarray ufuncs)
    negative(a[:],a[:])
    # must mark buffer content as dirty to update graph
    # (no explicit assignment occurred)
    a.dirty() 
