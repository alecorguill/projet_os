#!/bin/bash
gcc -Wall -Wextra -pthread -Isrc -DUSE_PTHREAD test/51-fibonacci.c -o 51_pthread
echo \#Fichier de comparaison de temps d execution sur fibonacci > graph.data
for((i=0;i<20;i++))
do
start=`date +%s%N`
start=$((start/1000))
./51 $i
end=`date +%s%N`
end=$((end/1000))
start2=`date +%s%N`
start2=$((start2/1000))
./51_pthread $i
end2=`date +%s%N`
end2=$((end2/1000))
runtime=$((end-start))
runtime2=$((end2-start2))
echo $i $runtime $runtime2 >> graph.data
done
 
gnuplot -e "set title 'Comparaison temps execution sur fibonacci';
set xlabel 'x';
set ylabel 'Temps execution de fibo(x) (us)';
plot 'graph.data' using 1:2 title 'thread' with linespoints ; replot 'graph.data' using 1:3 title 'pthread' with linespoints;" -p

rm 51_pthread
