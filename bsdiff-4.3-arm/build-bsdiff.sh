#!/bin/sh
set -e
#COMPILE=/opt/FriendlyARM/toolschain/4.5.1/bin
#CC=$COMPILE/arm-linux-gcc
#STRIP=$COMPILE/arm-linux-strip
CC=gcc
STRIP=strip
$CC -I ../bzip2-1.0.6 -O2 xmd5.cpp bsdiff.c -o bsdiff -L ../bzip2-1.0.6 -l bz2
$STRIP bsdiff

