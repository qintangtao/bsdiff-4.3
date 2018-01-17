#!/bin/sh
set -e
COMPILE=/opt/FriendlyARM/toolschain/4.5.1/bin
CC=$COMPILE/arm-linux-gcc
STRIP=$COMPILE/arm-linux-strip
$CC -I ../bzip2-1.0.6 -O2 xmd5.cpp bspatch.c -o bspatch -L ../bzip2-1.0.6 -l bz2
$STRIP bspatch

