#!/bin/bash
make distclean
rm aclocal.m4 autom4te.cache/ config.h.in -rf
rm configure
rm config.h.in~ 
rm src/gui/marshal.c src/gui/marshal.h
