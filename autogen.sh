#!/bin/bash
set -x
aclocal 					\
&& autoconf					\
&& libtoolize 	--copy --force --automake 	\
&& autoreconf --install				\
&& [ -d usr ] || mkdir usr			\
&& ./configure --prefix=`pwd`/usr/

