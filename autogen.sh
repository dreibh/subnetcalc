#!/bin/bash -e

CMAKE_OPTIONS=""
while [ $# -gt 0 ] ; do
   if [ "$1" == "-use-clang" ] ; then
      # Use these settings for CLang instead of GCC:
      export CXX=clang++
      export CC=clang
   elif [ "$1" == "-debug" ] ; then
      # Enable debugging build:
      CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=DEBUG"
   elif [ "$1" == "--" ] ; then
      break
   else
      echo >&2 "Usage: autogen.sh [-use-clang]"
      exit 1
   fi
   shift
done


rm -f CMakeCache.txt
echo "CMake options:${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=/usr $@ ."
cmake ${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=/usr $@ .

# ------ Obtain number of cores ---------------------------------------------
# Try Linux
cores=`getconf _NPROCESSORS_ONLN 2>/dev/null || true`
if [ "${cores}" == "" ] ; then
   # Try FreeBSD
   cores=`sysctl -a | grep 'hw.ncpu' | cut -d ':' -f2 | tr -d ' '`
fi
if [ "${cores}" == "" ] ; then
   cores="1"
fi

make -j${cores}
