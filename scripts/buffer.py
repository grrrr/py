# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""This is an example script for the py/pyext object's buffer support.

PD/Max buffers can be imported to and exported from numarray arrays.
For numarray see http://numeric.scipy.org
It will probably once be replaced by Numeric(3)
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
    a = pyext.Buffer(args[1])
    b = pyext.Buffer(args[2])

    # fastest method: slicing causes numarrays (mapped to buffers) to be created
    # note the c[:] - to assign contents you must assign to a slice
    c[:] = a[:]*b[:]  

def add(*args):
    c = pyext.Buffer(args[0])
    a = pyext.Buffer(args[1])
    b = pyext.Buffer(args[2])

    # this is also possible, but is probably slower
    # the + converts a into a numarray, the argument b is taken as a sequence
    # depending on the the implementation in numarray this may be as fast
    # as above or not
    c[:] = a+b  

def fadein(*args):
    a = pyext.Buffer(args[0])
    # inplace operations are ok
    a *= arange(len(a),type=Float32)/len(a)
