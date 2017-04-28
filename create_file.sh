#!/bin/bash

LIST=`ls $1`

for i in $LIST
do
./tmp/$i > ./correct/correct_$i.txt
done
