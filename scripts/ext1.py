import time,random
import pyext

# derive class from pyext.pyext
class testcl1(pyext._class): 
	  
# how many inlets and outlets?
	_inlets = 2
	_outlets = 2
	
# method for bang into inlet 1
	def bang_1(self):
		print "Hello"
		print time.time()
#		self._outlet(1,"bang")

# method for float into inlet 1
	def float_1(self,f):
		self._outlet(1,"one",f)

# method for float into any inlet
	def float_(self,ix,f):
		self._outlet(2,"any",ix,f)

# method for anything into any inlet
	def _anything_(self,ix,arg):
		print "ANYTHING (inlet",ix,"): ...", arg

# method for tag "hula" into any inlet
	def hula_(self,ix,arg):
		self._outlet(ix,"hula",arg)

def recv2(arg):
	print "RECV2",arg


class testcl2(pyext._class):
	_inlets=3
	_outlets=2

	recvname=""

	def recv1(self,arg):
		print "RECV1A",arg

	def __init__(self,name):
		print "__INIT__"
		self.recvname = name
		self._bind("recv1",self.recv1)
		self._bind("recv2",recv2)

#	def __del__(self):
#		print "__DEL__"
#		self._unbind(self.recvname,testcl2.recv1)

	def _anything_1(self,arg):
		self._priority(-1)
		for i in range(1,10):
			if self._shouldexit: break
			print i
			time.sleep(0.1)

	def _anything_2(self,arg):
		self._priority(1)
		self._send("send1",("hey",1,2))
