#!/bin/bash
set -x
aclocal 					\
&& autoconf					\
&& libtoolize 	--copy --force --automake 	\
&& autoreconf --install				\
