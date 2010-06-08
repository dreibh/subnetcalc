#!/bin/sh

./bootstrap && \
env CC=g++ ./configure --with-geoip $@ && \
( gmake || make )
