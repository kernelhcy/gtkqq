#!/bin/bash
set -x
aclocal 					\
&& autoconf					\
&& libtoolize 	--copy --force --automake 	\
&& autoreconf --install				\
&& [ -d usr ] || mkdir usr			\
&& ./configure --enable-debug --prefix=`pwd`/usr
