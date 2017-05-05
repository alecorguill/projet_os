#!/bin/bash


if [ $# -eq 1 ]
then
    gcc -Wall -Wextra -lpthread -DUSE_PTHREAD -Isrc test/$1*.c -o $1_pthread
    ./$1_pthread | sed /0x/d | cut -d":" -f1 | sort > ./correct/correct_$1.txt
    ./$1 | sed /0x/d | cut -d":" -f1 | sort > ./correct/$1.txt
elif [ $# -eq 2 ]
then
    gcc -Wall -Wextra -lpthread -DUSE_PTHREAD -Isrc test/$1*.c -o $1_pthread
    ./$1_pthread $2 | sort > ./correct/correct_$1.txt
    ./$1 $2 | sort > ./correct/$1.txt
 elif [ $# -eq 3 ]
then
    gcc -Wall -Wextra -lpthread -DUSE_PTHREAD -Isrc test/$1*.c -o $1_pthread
    ./$1_pthread $2 $3 | cut -d":" -f1 | sort > ./correct/correct_$1.txt
    ./$1 $2 $3 | cut -d":" -f1 | sort > ./correct/$1.txt
else echo "Problem nb arg"
fi

if [ "`diff -w -s ./correct/correct_$1.txt ./correct/$1.txt`" = "Les fichiers ./correct/correct_$1.txt et ./correct/$1.txt sont identiques" ]
then
echo -e "test_$1 \033[32mOK\033[0m"
else
echo -e "test_$1 \033[31mKO\033[0m"
fi


