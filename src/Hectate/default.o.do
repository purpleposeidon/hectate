

redo-ifchange ../vars
. ../vars

DEPFILE=$3.deps

redo-ifchange $1.cpp
$GCC -I../ -c $1.cpp -o $3 $DEPS $DEPFILE

read DEPS < $DEPFILE
DEPS=${DEPS#*:}
rm -f $DEPFILE

