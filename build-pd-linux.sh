#!/bin/sh  

. config-pd-linux.txt

make -f makefile.pd-linux &&
{ 
	if [ $INSTALL = "yes" ]; then
		echo Now install as root
		su -c "make -f makefile.pd-linux install"
	fi
}
