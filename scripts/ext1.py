import pyext

class testcl1(pyext.base):   
	def _inlets(self):
		return 2
	
	def _outlets(self):
		return 2

	def _bang_1(self):
		self._outlet(1,"Hello")
		print "bang method" 

	def _float_1(self,f):
		print "float method - ",f 



