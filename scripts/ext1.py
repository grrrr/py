import pyext

# derive class from pyext.base

class testcl1(pyext.base):   

# how many inlets?
	def _inlets(self):
		return 2
	
# how many outlets?
	def _outlets(self):
		return 2

# method for bang into inlet 1
	def _bang_1(self):
		self._outlet(1,"bang")

# method for float into inlet 1
	def _float_1(self,f):
		self._outlet(1,"one",f)

# method for float into any inlet
	def _float_(self,ix,f):
		self._outlet(2,"any",ix,f)

# method for anything into any inlet
	def _anything_(self,ix,arg):
		print "ANYTHING (inlet",ix,"):", arg

# method for tag "hula" into any inlet
	def _hula_(self,ix,arg):
		self._outlet(ix,"hula",arg)


class testcl2(pyext.base):

	def _anything_(self,ix,arg):
		print "HI! - ",arg
