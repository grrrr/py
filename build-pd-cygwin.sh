#!/bin/sh

. config-pd-cygwin.txt

make -f makefile.pd-cygwin &&
{
	if [ $INSTALL = "yes" ]; then
		make -f makefile.pd-cygwin install
	fi
}
