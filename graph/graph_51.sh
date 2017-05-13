#!/bin/bash
echo \#Fichier de comparaison de temps d execution sur fibonacci > graph.data
for((i=0;i<16;i++))
do
start=`date +%s%N`
start=$((start/1000))
../51 $i > /dev/null
end=`date +%s%N`
end=$((end/1000))
start2=`date +%s%N`
start2=$((start2/1000))
../51_pthread $i > /dev/null
end2=`date +%s%N`
end2=$((end2/1000))
runtime=$((end-start))
runtime2=$((end2-start2))
echo $i $runtime $runtime2 >> graph.data
done
for((i=16;i<19;i++))
do
start=`date +%s%N`
start=$((start/1000))
../51 $i > /dev/null
end=`date +%s%N`
end=$((end/1000))
runtime=$((end-start))
echo $i $runtime >> graph.data
done
gnuplot -e "set title 'Comparaison temps execution sur fibonacci';
set xlabel 'x';
set ylabel 'Temps execution de fibo(x) (us)';
plot 'graph.data' using 1:2 title 'thread' with linespoints ; replot 'graph.data' using 1:3 title 'pthread' with linespoints;" -p
