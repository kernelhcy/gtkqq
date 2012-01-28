#!/bin/bash
set -x
aclocal 					\
&& autoconf					\
&& libtoolize 	--copy --force --automake 	\
&& autoreconf --install				\
&& [ -d usr ] || mkdir usr			\
&& ./configure --enable-debug --enable-proxy --prefix=`pwd`/usr
