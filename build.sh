#!/bin/sh

set -xe

CC="gcc"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb"
LIBS="`pkg-config --cflags --libs sdl2` -lm"
SOURCE="ked.c v2.c"
OBJS="ked.o v2.o"

$CC $CFLAGS -c $SOURCE
$CC $OBJS $LIBS -o ked
rm *.o

set +xe
