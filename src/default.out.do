

redo-ifchange vars
. ./vars

redo-ifchange Hectate/words.h Hectate/words.cpp

OBJS=$(find Hectate/ -name \*.cpp | sed "s/\.cpp$/\.o/")
redo-ifchange $OBJS $1.cpp

$GCC -I./ $1.cpp $OBJS -o $3 $LIBS

