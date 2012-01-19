#!/bin/bash
make distclean 2>/dev/null
rm aclocal.m4 autom4te.cache/ config.h.in m4 usr build-aux -rf 2>/dev/null
rm configure 2>/dev/null
(rm -rf Packages/rpm/rpmbuild 2>/dev/null)
(find . -name Makefile.in -exec rm {} \;)
