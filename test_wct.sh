#!/bin/bash

# Run the intersection program using different algorithms on different
# input sizes, from FROM_SIZE to TO_SIZE using step STEP_SIZE. Each
# measurement is repeated NREPS times and then averages. Results are
# stored in suitably-named text files.
#
# Run this script as:
#
# ./test_wct.sh
#
# Last modified 2022-10-01 by Moreno Marzolla

# number of replications
NREPS=5
# initial n. of intervals
FROM_SIZE=1000000
# final n. of intervals
TO_SIZE=80000000
# step
STEP_SIZE=5000000
# where to place test results
OUT_DIR=test_results

mkdir -p ${OUT_DIR}

for ALGO in thrust_seq thrust_omp thrust_cuda; do
    EXE="./intersections_${ALGO}"

    if [ ! -f ${EXE} ]; then
        echo "FATAL: Missing executable \"${EXE}\""
        exit 1
    fi

    FNAME="${OUT_DIR}/`hostname`_${ALGO}_wct.txt"
    echo "# Machine: `hostname`" > ${FNAME}
    echo "# Algorithm: ${ALGO}" >> ${FNAME}
    echo "# Date: `date`" >> ${FNAME}
    echo "# n_intervals time_sec" > ${FNAME}
    for SIZE in `seq $FROM_SIZE $STEP_SIZE $TO_SIZE`; do
        echo -n "$ALGO $SIZE "
        TSUM=0
        for REP in `seq 1 $NREPS`; do
            TIME=$(${EXE} -N ${SIZE} | grep -i "Intersection time" | egrep -o "[[:digit:]\.]+")
            echo -n "$REP"
            TSUM=$(echo "$TIME + $TSUM" | bc -l)
        done
        TAVE=$(echo "$TSUM / $NREPS" | bc -l)
        echo "$SIZE $TAVE" >> ${FNAME}
        echo " $TAVE"
    done
    echo
done
