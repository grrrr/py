# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""Several functions to show the py script functionality"""

import sys

print "Script initialized"

try:
	print "Script arguments: ",sys.argv
except:
	print 

def numargs(*args):   # variable argument list
	"""Return the number of arguments"""
	return len(args)

def strlen(arg):   
	"""Return the string length"""
	return len(arg)


def strcat(*args):
	"""Concatenate several symbols"""
	s = ""
	for si in args:
		s += str(si)
	return s


def addall(*args):   # variable argument list
	s = 0
	for si in args:
		s += si
	return s


def ret1():
	return 1,2,3,4


def ret2():
	return "sd","lk","ki"


def ret3():
	return ["sd","lk","ki"]

