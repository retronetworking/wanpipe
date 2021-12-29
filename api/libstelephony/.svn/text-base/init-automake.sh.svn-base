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

echo "Automake Init Done"
echo 

echo "Please run " 
echo " -> ./configure --prefix=/usr"
echo " -> make "
echo " -> make install"           

