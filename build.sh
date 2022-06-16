#!/bin/sh

set -xe

CC="gcc"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb"
LIBS="`pkg-config --cflags --libs sdl2` -lm"
SOURCE="rogueban.c v2.c"
OBJS="rogueban.o v2.o"

$CC $CFLAGS -c $SOURCE
$CC $OBJS $LIBS -o rogueban