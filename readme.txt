py/pyext - python script objects for PD (and MaxMSP... once, under MacOSX and Windows)

Copyright (c)2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.
Visit https://www.paypal.com/xclick/business=t.grill%40gmx.net&item_name=pyext&no_note=1&tax=0&currency_code=EUR

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
see http://www.parasitaere-kapazitaeten.net/ext


Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.cpp, main.h, modmeth.cpp, pyargs.cpp, register.cpp: base class
- py.cpp: py object
- pyext.cpp, pyext.h, clmeth.cpp, bound.cpp: pyext object

----------------------------------------------------------------------------

Goals/features of the package:

Access the flexibility of the python language in PD and MaxMSP


PD - Load it as i library with e.g. "pd -lib py -path scripts"

Max/MSP - Wait for Windows or a Mach-O MacOSX version. MacOS9 doesn't want it.


Check out the sample patches and scripts


Description:

With the py object you can load python modules and execute the functions therein.
With the pyext you can use python classes to represent full-featured pd/Max message objects.
Multithreading (detached methods) is supported for both objects.
You can send messages to named objects or receive (with pyext) with Python methods.


Known bugs:
- The TCL/TK help patch is not usable under OSX.
- With the standard PD distribution, threaded py scripts will cause "Stack overflows" under some circumstances
	(the devel_0_37 cvs branch of PD contains the relevant fixes to avoid that)

----------------------------------------------------------------------------

The py/pyext package should run with Python version >= 2.1.
It has been thoroughly tested with version 2.2 and 2.3


The package should at least compile (and is tested) with the following compilers:


PD @ Windows:
-------------
o Borland C++ 5.5 (free): edit "config-pd-bcc.txt" & run "build-pd-bcc.bat" 

o Microsoft Visual C++ 6: usr "py.dsp" or edit "config-pd-msvc.txt" & run "build-pd-msvc.bat" 


PD @ linux:
-----------
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 


PD @ MacOSX:
---------------------
You'll need to have Python installed as a framework. 
This is the default with Panther - otherwise, all newer Python source distributions are buildable as a darwin framework 
( ./configure --enable-framework=/System/Library/Frameworks && make && make installframework )

o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 


----------------------------------------------------------------------------

Version history:

0.1.4:
- ADD: better (and independent) handling of inlet and outlet count (as class variables or dynamically initialized in __init__)
- FIX: many memory leaks associated to ***GetItem stuff (big thanks to sven!)
- FIX: set "this" memory in object after reloading script
- ADD: _getvalue,_setvalue to access PD values
- FIX: don't shout when Python script returns PyNone
- ADD: alias creation names pyext. and pyx. take the script name also for the class name

0.1.3:
- FIX: class variables are now set atomic if parameter list has only 1 element
- ADD: introduced shortcut "pyx" for pyext.
- ADD: arguments to the pyext class are now exposed as attributes "args"
- FIX: parameters to Python functions are treated as integers when they can be.
- ADD: inlet and outlet count can be given for pyext, python _inlet and _outlet members are ignored then
- FIX: crash if script or class names are non-strings
- FIX: long multi-line doc strings are now printed correctly
- FIX: message "doc+" for class/instance __doc__ now working
- FIX: improved/debugged handling of reference counts
- FIX: _pyext._send will now send anythings if feasible
- CHANGE: no more finalization - it's really not necessary...
- FIX: calling from unregistered threads (like flext helper thread) now works

0.1.2:
- CHANGE: updates for flext 0.4.1 - method registering within class scope
- FIX: bugs in bound.cpp (object_free calls)
- FIX: bug with threaded methods along with flext bug fix.
- ADD: map Python threads to system threads
- ADD: shut down the Python interpreter appropriately
- CHANGE: use flext timer and bind functionality
- ADD: attribute functionality
- ADD: dir and dir+ methods for Python dictionary listing
- ADD: get and set methods for Python class attributes

0.1.1:
- CHANGE: updates for flext 0.4.0
- FIX: crash when module couldn't be loaded
- FIX: GetBound method (modmeth.cpp, line 138) doesn't exist in flext any more
- FIX: deadlock occured when connecting to py/pyext boxes in non-detached mode
- ADD: current path and path of the canvas is added to the python path
- FIX: path is not added to python path if already included

0.1.0:
- completely reworked all code
- added class functionality for full-featured objects and renamed the merge to pyext
- enabled threads and made everything thread-safe ... phew!
- using flext 0.3.2
- pyext now gets full python path
- python's argv[0] is now "py" or "pyext"
- etc.etc.

0.0.2:
- fixed bug when calling script with no function defined (thanks to Ben Saylor)
- cleaner gcc makefile

0.0.1:
- using flext 0.2.1

---------------------------------------------------------------------------

TODO list:

general:
- Documentation and better example patches
- better error reporting for runtime errors

features:
- enable multiple interpreters?
- make a pygui object where Tkinter draws to the PD canvas...
- stop individual threads
- Python type for symbols

tests:
- check for python threading support

bugs:
- named (keyword) arguments are not supported
- currently no support for Python threads

