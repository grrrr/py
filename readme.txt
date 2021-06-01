py/pyext - python script objects for PD and Max/MSP

This fork by SOPI research group (https://sopi.aalto.fi) implements support for Python 3 as well as Conda Python installations.
It was developed for use with GANSpaceSynth (https://github.com/SopiMlab/GANSpaceSynth) and our Deep Learning with Audio course (https://github.com/SopiMlab/DeepLearningWithAudio).

See also the original repository: https://github.com/grrrr/py

Copyright (c)2002-2020 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

----------------------------------------------------------------------------

You need to have Python installed on your system for the py/pyext external to work.
We recommend using Conda (https://conda.io).

SopiMlab/py has been tested with Python 3.7-3.8 and 2.7.

Check out the sample patches and scripts

----------------------------------------------------------------------------

INSTALLATION
============

We have detailed tutorials for setting up GANSpaceSynth with Pd in a Conda environment:
https://github.com/SopiMlab/DeepLearningWithAudio/tree/master/utilities/pyext-setup

Known issues:

- We haven't been able to build on Windows
- Max/MSP is untested

Contributions are welcome!

----------------------------------------------------------------------------

DESCRIPTION
===========

With the py object you can load python modules and execute the functions therein.
With the pyext you can use python classes to represent full-featured pd/Max message objects.
Multithreading (detached methods) is supported for both objects.
You can send messages to named objects or receive (with pyext) with Python methods.

----------------------------------------------------------------------------

BUILDING from source
====================

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
See https://github.com/SopiMlab/flext

TODO: Document our changes to the build configuration. For now, the build.py script from https://github.com/SopiMlab/DeepLearningWithAudio/blob/master/utilities/pyext-setup/build.py can give you some hints

Pure data - any platform supporting gcc-compatible compilers
------------------------------------------------------------

The pd-lib-builder project (https://github.com/pure-data/pd-lib-builder) is used to compile the project.
A git subtree of this project is already present.

The compilation is done using the GNU make tool and it will need additional information about the location of the flext source files, and possibly, Pure data, if a specific version should be used.

This could be an example:
make CPPFLAGS="-I ../flext/source" PDDIR="../../pure-data"

For OS X, further flags can be needed:
CFLAGS="-mmacosx-version-min=10.9" LDFLAGS="-mmacosx-version-min=10.9"


pd/Max - Windows - Microsoft Visual C, Borland C++, MinGW:
----------------------------------------------------------
Please see further setup instructions at https://github.com/grrrr/py

pd/Max - OSX/Linux - GCC:
-------------------------
Please see further setup instructions at https://github.com/grrrr/py
----------------------------------------------------------------------------
Please see further setup instructions at https://github.com/grrrr/py

