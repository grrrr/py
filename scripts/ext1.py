"""This is an example script for the py/pyext object's send/receive functionality.

There are several classes exposing py/pyext features:
- sendrecv: A class receiving messages and sending them out again

"""

import pyext

#################################################################

def recv_gl(arg):
	"""This is a global receive function, it has no access to class members."""
	print "GLOBAL",arg

class sendrecv(pyext._class):
	"""Example of a class which receives and sends messages

	It has two creation arguments: a receiver and a sender name.
	There are no inlets and outlets.
	Python functions (one global function, one class method) are bound to PD's or Max/MSP's receive symbols.
	The class method sends the received messages out again.
	"""


	# no inlets and outlets
	_inlets=0
	_outlets=0

	recvname=""   
	sendname=""

	def recv(self,arg):
		"""This is a class-local receive function, which has access to class members."""

		# print some stuff
		print "CLASS",self.recvname,arg

		# send data to specified send address
		pyext._send(self.sendname,arg)


	def __init__(self,rname,sname):
		"""Class constructor"""

		# store sender/receiver names
		self.recvname = rname
		self.sendname = sname

		# bind functions to receiver names
		# both are called upon message 
		self._bind(self.recvname,self.recv)
		self._bind(self.recvname,recv_gl)


	def __del__(self):
		"""Class destructor"""

		# you can but you don't need to
		# unbinding is automatically done at destruction
		# you can also comment the _unbind lines
		self._unbind(self.recvname,self.recv)
		self._unbind(self.recvname,recv_gl)

		pass
