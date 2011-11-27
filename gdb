#!/bin/sh

#No rlwrap. :(

PROGRAM=src/shell.out

./build $PROGRAM && gdb --args $PROGRAM $@

