"""Several functions to show the py script functionality"""

import sys

print "Script scr1.py initialized"

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
		s += si
	return s


print "DIR",dir()






