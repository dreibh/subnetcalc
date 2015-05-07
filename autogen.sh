#!/bin/sh

./bootstrap && \
./configure --with-geoip $@ && \
( gmake || make )
