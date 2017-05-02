#!/bin/bash

if [ $# -eq 1 ]
then
    ./$1 | sed /0x/d | sort> ./correct/$1.txt
    
elif [ $# -eq 2 ]
then
    ./$1 $2 | sed /0x/d | sort > ./correct/$1.txt
elif [ $# -eq 2 ]
then
    ./$1 $2 $3 | sed /0x/d | sort > ./correct/$1.txt
else echo "Problem nb arg"
fi

if [ "`diff -w -s ./correct/correct_$1.txt ./correct/$1.txt`" = "Les fichiers ./correct/correct_$1.txt et ./correct/$1.txt sont identiques" ]
then
echo -e "test_$1 \033[32mOK\033[0m"
else
echo -e "test_$1 \033[31mKO\033[0m"
fi


