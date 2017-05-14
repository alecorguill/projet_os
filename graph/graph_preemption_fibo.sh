#!/bin/bash
cd ..
gcc -Wall -Wextra -DPREEMPTION -lpthread -Isrc -g -O0 src/thread.c test/51*.c -o 51_preemption
gcc -Wall -Wextra -lpthread -Isrc -g -O0 src/thread.c test/51*.c -o 51

echo \#Fichier de comparaison de temps fibo avec sans preemption > graph.data
for((i=0;i<25;i++))
do
start=`date +%s%N`
start=$((start/1000))
./51 $i > /dev/null
end=`date +%s%N`
end=$((end/1000))
start2=`date +%s%N`
start2=$((start2/1000))
./51_preemption $i > /dev/null
end2=`date +%s%N`
end2=$((end2/1000))
runtime=$((end-start))
runtime2=$((end2-start2))
echo $i $runtime $runtime2 >> graph.data
done

gnuplot -e "set title 'Comparaison temps execution sur le test 21';
set xlabel 'x';
set ylabel 'Temps execution de fibo(x)(us)';
plot 'graph.data' using 1:2 title 'Sans préemption' with linespoints ; replot 'graph.data' using 1:3 title 'Avec préemption' with linespoints;" -p

rm 51 51_preemption graph.data
cd graph

