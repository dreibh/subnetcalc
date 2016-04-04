#!/bin/sh

if [ -e /usr/bin/dpkg-buildflags ] ; then
   export CPPFLAGS=`dpkg-buildflags --get CPPFLAGS`
   export CFLAGS=`dpkg-buildflags --get CFLAGS`
   export CXXFLAGS=`dpkg-buildflags --get CXXFLAGS`
   export LDFLAGS=`dpkg-buildflags --get LDFLAGS`
fi

./bootstrap && \
./configure --with-geoip $@ && \
( gmake || make )
