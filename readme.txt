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
o Borland C++ 5.5 (free): run "make -f makefile.bcc" - makefile is no real make but works
o Microsoft Visual C++ 6: use "py.dsp" project file 

- pd - linux:
o GCC for linux: run "make -f makefile.pd-linux" 

- Max/MSP - MacOS:
o Metrowerks CodeWarrior V6: use "py" project file 

----------------------------------------------------------------------------

Goals/features of the package:

- access the flexibility of the python language in PD/MaxMSP


Description:

- 

----------------------------------------------------------------------------

Version history:

0.0.1:
- using flext 0.2.0



---------------------------------------------------------------------------


TODO list:

general:
- Documentation and better example patches
- cleaner makefile for PD/Borland C++

features:
- analyze returned lists
- set method for called function (set is not passed to the script then)
- 

tests:

bugs:
- script is unusable once an error occured
