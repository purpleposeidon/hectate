

OUTS=$(echo *.cpp | sed 's/\.cpp/\.out/g')
redo-ifchange $OUTS


