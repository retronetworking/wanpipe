#!/bin/sh

PWD=$(pwd)
eval "cd . && /bin/sh $PWD/missing --run aclocal"
if [ $? -ne 0 ]; then
	exit $?
fi
eval "cd . && /bin/sh $PWD/missing --run automake --gnu"
if [ $? -ne 0 ]; then
	exit $?
fi
eval "cd . && /bin/sh $PWD/missing --run autoconf"
if [ $? -ne 0 ]; then
	exit $?
fi

cd sample_c
if [ -f Makefile ]; then
	rm -f Makefile
fi
ln -s Makefile.Linux Makefile

cd ..

cd sample_cpp
if [ -f Makefile ]; then
	rm -f Makefile
fi
ln -s Makefile.Linux Makefile

echo "Automake Init Done"
echo 

echo "Please run " 
echo " -> ./configure --prefix=/usr"
echo " -> make "
echo " -> make install"           

