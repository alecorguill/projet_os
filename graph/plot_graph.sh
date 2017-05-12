#!/bin/bash

rm -f *~
FILES=`ls | grep graph_`
cd ..
make check > /dev/null
cd graph
if [ $# -eq 1 ]
then
./graph_$1.sh
else
for i in $FILES
do
./$i
done
fi
cd ..
make clean > /dev/null
cd graph
