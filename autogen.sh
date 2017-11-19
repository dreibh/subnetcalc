#!/bin/bash

rm -f CMakeCache.txt
cmake -DCMAKE_INSTALL_PREFIX=/usr $@ .

# ------ Obtain number of cores ---------------------------------------------
# Try Linux
cores=`getconf _NPROCESSORS_ONLN 2>/dev/null`
if [ "$cores" == "" ] ; then
   # Try FreeBSD
   cores=`sysctl -a | grep 'hw.ncpu' | cut -d ':' -f2 | tr -d ' '`
fi
if [ "$cores" == "" ] ; then
   cores="1"
fi

gmake -j$cores || make -j$cores
