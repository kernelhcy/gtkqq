#!/bin/bash
echo "aclocal..."
aclocal
echo "autoconf..."
autoconf
echo "autoheader..."
autoheader
echo "automake..."
automake -a
./configure --enable-debug --prefix=/home/hcy/tmp/gtkqq
