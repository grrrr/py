xsample - extended sample objects for Max/MSP and pd (pure data)
version 0.2.2

Copyright (c) 2001,2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.


Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.h,main.cpp: base class definition for all the other objects
- record.cpp: xrecord~
- play.cpp: xplay~
- groove.cpp: xgroove~

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

- pd - Windows:
o Borland C++ 5.5 (free): run "make -f makefile.bcc" - makefile is no real make but works
o Microsoft Visual C++ 6: use "xsample.dsp" project file - due to a compiler bug the optimization using templates is not functional

- pd - linux:
o GCC for linux: run "make -f makefile.pd-linux" - GCC 2.95.2 dies with template optimization turned on 

- Max/MSP - MacOS:
o Metrowerks CodeWarrior V6: use "xsample" project file - using dsp method redirection via static functions

----------------------------------------------------------------------------

Goals/features of the package:

- portable and effective sample recording/playing objects for pd and Max/MSP
- MSP-like groove~ object for PD 
- message- or signal-triggered recording object with mix-in capability
- avoid the various bugs of the original MSP2 objects
- multi-channel capability 
- live update of respective buffer/array object
- switchable 4-point-interpolation for xplay~/xgroove~ object

----------------------------------------------------------------------------

Version history:

0.2.2:
- using flext 0.2.0
- xrecord~ for PD: new flext brings better graphics update behavior
- xrecord~: recording position doesn't jump to start when recording length is reached
- fixed bug with refresh message (min/max reset)
- xgroove~: position (by pos message) isn't sample rounded anymore
- reset/refresh messages readjust dsp routines to current buffer format (e.g. channel count)
- corrected Max/MSP assist method for multi-channel
- fixed xplay~ help method 
- changed syntax to x*~ [channels=1] [buffer] for future enhancements (MaxMSP only, warning for old syntax)
- fixed small bug concerning startup position in xgroove~ and xrecord~

0.2.1:
- no leftmost float inlet for position setting - use pos method
- changed dsp handling for flext 0.1.1 conformance
- workarounds for buggy/incomplete compilers
- prevent buffer warning message at patcher load (wait for loadbang)
- fixed bug: current pos is reset when changing min or max points

0.2.0: 
- first version for flext

---------------------------------------------------------------------------


TODO list:

general:
- Documentation and better example patches
- cleaner makefile PD/Borland C++
- makefile and build for PD/GCC
- eventually make use of resource files for text items

features:
- crossfading loop zone for xgroove~
- multi-buffer handling (aka multi-channel for pd)
- vasp handling
- performance comparison to respective PD/Max objects
- anti-alias filter? (possible?)

tests:
- test graphics update behavior in Max (all done automatically ?)
- reconsider startup sequence of set buffer,set units,set sclmode,set pos/min/max

bugs:
no unfixed known