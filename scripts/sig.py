# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""This is an example script for the py/pyext signal support.

For numarray see http://numeric.scipy.org
It will probably once be replaced by Numeric(3)
"""

try:
    import pyext
except:
    print "ERROR: This script must be loaded by the PD/Max py/pyext external"

try:
    # try to use JIT support
    import psyco
    psyco.full()
except:
    pass

import sys

try:    
    import numarray
except:
    print "Failed importing numarray module:",sys.exc_value


class gain(pyext._class):
    """Just a simple gain stage"""
    
    gain = 0

    def float_1(self,g):
        self.gain = g

    def _signal(self):
        # Multiply input vector by gain and copy to output
        self._outvec(0)[:] = self._invec(0)*self.gain
