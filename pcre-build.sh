#!/bin/sh
# hack around make stupidity, sorry

cd pcre

./configure \
	--prefix=/usr \
	--disable-cpp \
	--enable-utf8 \
	--disable-shared \
	--enable-static \
	CFLAGS="-O3 -fPIC" && make
