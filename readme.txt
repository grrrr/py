py - python script object for PD and MaxMSP
version 0.0.1

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.


Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.cpp: all the stuff

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

- pd - Windows:
o Borland C++ 5.5 (free): edit & run "make -f makefile.bcc" 
o Microsoft Visual C++ 6: edit "py.dsp" project file 

- pd - linux:
Python doesn't provide a shared lib by default - static linking produces huge externals...
o GCC for linux: edit & run "make -f makefile.pd-linux" 

- Max/MSP - MacOS:
The source compiles and links but Max dies on first call of the Python API
o CodeWarrior Pro: 
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

0.0.1:
- using flext 0.2.1


---------------------------------------------------------------------------


TODO list:

general:
- Documentation and better example patches

features:

tests:

bugs:
