import sys

print "Script scr1.py initialized"

try:
	print "Script arguments: ",sys.argv
except:
	print 

def numargs(*args):   # variable argument list
#	print "Function args" args

	return len(args)

def strlen(arg):   
	return len(arg)


def strcat(*args):   
	print "Type",type(args)
	s = ""
	for si in args:
		print "Typei",type(si)
		s += si
	return s






