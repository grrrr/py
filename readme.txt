py/pyext - python script objects for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.

Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.cpp, main.h: base class
- py.cpp: py object
- pyext.cpp: pyext object

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

- pd - Windows:
o Borland C++ 5.5 (free): edit & run "make -f makefile.bcc" 
o Microsoft Visual C++ 6: edit "py.dsp" project file 

- pd - linux:
Python doesn't provide a shared lib by default - static linking produces huge externals
Ok, debian is an exception...
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 

- Max/MSP - MacOS:
The source compiles and links but Max dies on first call of the Python API
o CodeWarrior Pro: edit "py.cw" project file 
o MPW-PR and GUSI: #undef HAVE_USABLE_WCHAR_T and HAVE_WCHAR_H in pyconfig.h

----------------------------------------------------------------------------

Goals/features of the package:

- access the flexibility of the python language in PD/MaxMSP

Description:

- you can load python modules and execute the functions therein
- the python scripts are searched within the pd path (specified with -path option)
- different py objects can share the same modules, hence creation arguments only apply upon (re)load of the first instance
- list, float, symbol messages to the script are transmitted without the header (only the message element(s))
- multi-element results (tuple, list) from the python script are prepended by list

----------------------------------------------------------------------------

Version history:

0.1.0:
- using flext 0.3.2
- another bugfix for undefined function
- added pyext for full-featured objects
- py/pyext now get full python path
- make external name python argv[0]
- enabled threads and made everything thread-safe ... phew!

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
- introduce a _shouldexit attribute for thread termination (or raise a signal?)

tests:

bugs:
- the python interpreter won't be unloaded due to some bug at re-initialization
- fast message repetitions in detached mode make python crash

