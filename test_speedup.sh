#!/bin/bash

#
# Run this script as:
#
# ./test_speedup.sh
#
# Last modified 2022-12-06 by Moreno Marzolla
#

# number of replications
NREPS=5
# n. of intervals
SIZE=50000000
# where to place test results
OUT_DIR=test_results

mkdir -p ${OUT_DIR}

for ALGO in omp; do
    EXE="./intersections_thrust_${ALGO}"

    if [ ! -f ${EXE} ]; then
        echo "FATAL: Missing executable \"${EXE}\""
        exit 1
    fi

    FNAME="${OUT_DIR}/`hostname`_${ALGO}_speedup.txt"
    echo "# Machine: `hostname`" > ${FNAME}
    echo "# Algorithm: ${ALGO}" >> ${FNAME}
    echo "# Date: `date`" >> ${FNAME}
    echo "# N of intervals: $SIZE" >> ${FNAME}
    echo "# Legend:" >> ${FNAME}
    echo "# P time_sec" >> ${FNAME}
    NPROC=`cat /proc/cpuinfo | grep processor | wc -l`
    for P in `seq 1 $NPROC` ; do
        echo -n "$ALGO $P "
        TIME=$(OMP_NUM_THREADS=$P ${EXE} -r ${NREPS} -N ${SIZE} | grep -i "Intersection time" | egrep -o "[[:digit:]\.]+")
        echo "$P $TIME" >> ${FNAME}
        echo "$TIME"
    done
done
