#!/bin/bash
echo \#Fichier de comparaison de temps d execution sur création destruction recursif de thread > graph.data
for((i=0;i<200;i+=5))
do
start=`date +%s%N`
start=$((start/1000))
../22 $i > /dev/null
end=`date +%s%N`
end=$((end/1000))
start2=`date +%s%N`
start2=$((start2/1000))
../22_pthread $i > /dev/null
end2=`date +%s%N`
end2=$((end2/1000))
runtime=$((end-start))
runtime2=$((end2-start2))
echo $i $runtime $runtime2 >> graph.data
done

gnuplot -e "set title 'Comparaison temps execution sur le test 22';
set xlabel 'nombre de threads';
set ylabel 'Temps execution (us)';
plot 'graph.data' using 1:2 title 'thread' with linespoints ; replot 'graph.data' using 1:3 title 'pthread' with linespoints;" -p
