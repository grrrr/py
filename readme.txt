py/pyext - python script objects for PD (and MaxMSP... once, under MacOSX and Windows)

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.

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
Max/MSP - Wait for Windows or MacOSX version. MacOS9 doesn't want it.


Check out the sample patches and scripts


Description:

With the py object you can load python modules and execute the functions therein.
With the pyext you can use python classes to represent full-featured pd/Max message objects.
Multithreading (detached methods) is supported for both objects.
You can send messages to named objects or receive (with pyext) with Python methods.


Known bugs:
There are problems with importing the "time" module, experienced with Python 2.2.2 under OSX.
Therefore, some help patches may not be functional.

----------------------------------------------------------------------------

The py/pyext package should run with Python version >= 2.1.
It has been thoroughly tested with version 2.2


The package should at least compile (and is tested) with the following compilers:

PD @ Windows:
-------------
o Borland C++ 5.5 (free): edit "config-pd-bcc.txt" & run "build-pd-bcc.bat" 

o Microsoft Visual C++ 6: usr "py.dsp" or edit "config-pd-msvc.txt" & run "build-pd-msvc.bat" 

PD @ linux:
-----------
Python doesn't provide a shared lib by default - static linking produces huge externals
Ok, debian is an exception... the precompiled binary is for debian, therefore.

o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 

PD @ MacOSX:
---------------------

o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 

----------------------------------------------------------------------------

Version history:

0.1.2:
- CHANGE: updates for flext 0.4.1 - method registering within class scope

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

features:
- enable multiple interpreters?
- make a pygui object where Tkinter draws to the PD canvas...
- stop individual threads

tests:
- check for python threading support

bugs:
- the python interpreter can't be unloaded due to some bug at re-initialization
- named (keyword) arguments are not supported

