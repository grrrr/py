#!/bin/sh  

. config-pd-darwin.txt

make -f makefile.pd-darwin &&
{ 
	if [ $INSTPATH != "" ]; then
		echo Now install as root
		sudo make -f makefile.pd-darwin install
	fi
}
