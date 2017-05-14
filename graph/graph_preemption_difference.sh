#!/bin/bash
cd ..
gcc -Wall -Wextra -DPREEMPTION -lpthread -Isrc -g -O0 src/thread.c test/64*.c -o 64_preemption
gcc -Wall -Wextra -lpthread -Isrc -g -O0 src/thread.c test/64*.c -o 64

echo \#Fichier de comparaison de temps fibo avec sans preemption > graph.data
for((i=10;i<20000;i += 1000))
do
start=`date +%s%N`
start=$((start/1000))
./64 $i > /dev/null
end=`date +%s%N`
end=$((end/1000))
start2=`date +%s%N`
start2=$((start2/1000))
./64_preemption $i > /dev/null
end2=`date +%s%N`
end2=$((end2/1000))
runtime=$((end-start))
runtime2=$((end2-start2))
echo $i $runtime $runtime2 >> graph.data
done

gnuplot -e "set title 'Comparaison temps execution sur le test 64';
set xlabel 'Valeur à dépasser';
set ylabel 'Temps execution';
plot 'graph.data' using 1:2 title 'Sans préemption' with linespoints ; replot 'graph.data' using 1:3 title 'Avec préemption' with linespoints;" -p

rm 64 64_preemption graph.data
cd graph

