#!/bin/bash

#
# Run this script as:
#
# ./test_speedup.sh
#
# Last modified 2024-02-13 by Moreno Marzolla
#

# number of replications
NREPS=5
# n. of intervals
SIZE=100000000
# where to place test results
OUT_DIR=test_results

mkdir -p ${OUT_DIR}

for ALGO in omp ; do
    EXE="./intersections_${ALGO}"

    if [ ! -f ${EXE} ]; then
        echo "FATAL: Missing executable \"${EXE}\""
        exit 1
    fi

    FNAME="${OUT_DIR}/`hostname`_${ALGO}_speedup.txt"
    echo "# Machine: `hostname`" > ${FNAME}
    echo "# Algorithm: ${ALGO}" >> ${FNAME}
    echo "# N. of replications: ${NREPS}" >> ${FNAME}
    echo "# Date: `date`" >> ${FNAME}
    echo "# N of intervals: $SIZE" >> ${FNAME}
    echo "# Legend:" >> ${FNAME}
    echo "# P time_sec" >> ${FNAME}
    NPROC=`cat /proc/cpuinfo | grep processor | wc -l`
    for P in `seq 1 $NPROC` ; do
        echo -n "$ALGO $P/$NPROC "
        TIME=$(OMP_NUM_THREADS=$P ${EXE} -r ${NREPS} -N ${SIZE} | grep -i "Intersection time" | egrep -o "[[:digit:]]+\.[[:digit:]]+")
        echo "$P $TIME" >> ${FNAME}
        echo "$TIME"
    done
done
