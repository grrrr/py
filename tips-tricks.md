pd-lib-builder cheatsheet
=========================


## cross-compiling W32 binaries from linux

I'm using the following to cross-compile W32 binaries on my Debian/64bit system,
using `mingw-w64`.

Assuming you have unzipped a W32 package for Pd into `${WINPDPATH}`, run:

    make system=Windows pdbinpath="${WINPDPATH}/bin/" pdincludepath="${WINPDPATH}/src/" CC=i686-w64-mingw32-gcc

(if the project uses C++, you might also need to sed `CXX=i686-w64-mingw32-g++`)
