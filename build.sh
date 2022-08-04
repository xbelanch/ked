#!/bin/bash 

set -xe

out="ked"
cc="/usr/bin/gcc"
cflags="-Wall -Wextra -std=c11 -pedantic -ggdb"
libs="`pkg-config --cflags --libs sdl2` -lm"
src=( $(ls *.c) )
$cc $cflags -c ${src[*]}
objs=( $(ls *.o) )
$cc ${objs[*]} $libs -o $out
rm ${objs[*]}

set +xe
