#!/bin/bash

# run on 1 machine * 28 process, feel free to change it!
if [ $2 -lt 500 ] ; then
    srun -N 1 -n 1 $*

else if [ $2 -lt 2000 ] ; then
    srun -N 1 -n 2 --cpu-bind=verbose,map_cpu:0,2 $*

else if [ $2 -lt 5000 ] ; then
    srun -N 1 -n 8 --cpu-bind=verbose,map_cpu:0,2,4,6,8,10,12,14 $*

else if [ $2 -lt 25000 ] ; then
    srun -N 1 -n 14 --cpu-bind=verbose,map_cpu:0,2,4,6,8,10,12,14,16,18,20,22,24,26 $*

else if [ $2 -lt 80000 ] ; then
    srun -N 1 -n 28 --cpu-bind=verbose,map_cpu:0,2,4,6,8,10,12,14,16,18,20,22,24,26,1,3,5,7,9,11,13,15,17,19,21,23,25,27 $*

else if [ $2 -lt 30000000 ] ; then
    srun -N 2 -n 56 --cpu-bind=verbose,map_cpu:0,2,4,6,8,10,12,14,16,18,20,22,24,26,1,3,5,7,9,11,13,15,17,19,21,23,25,27 $*

else
    srun -N 2 -n 40 --cpu-bind=verbose,map_cpu:0,2,4,6,8,10,12,14,16,18,1,3,5,7,9,11,13,15,17,19 $*

fi
fi
fi
fi
fi
fi