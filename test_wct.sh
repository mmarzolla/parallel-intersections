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
# Last modified 2022-12-06 by Moreno Marzolla

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

for ALGO in seq omp cuda; do
    EXE="./intersections_thrust_${ALGO}"

    if [ ! -f ${EXE} ]; then
        echo "FATAL: Missing executable \"${EXE}\""
        exit 1
    fi

    FNAME="${OUT_DIR}/`hostname`_${ALGO}_wct.txt"
    echo "# Machine: `hostname`" > ${FNAME}
    echo "# Algorithm: ${ALGO}" >> ${FNAME}
    echo "# Date: `date`" >> ${FNAME}
    echo "# Legend:" >> ${FNAME}
    echo "# n_intervals time_sec" >> ${FNAME}
    for SIZE in `seq $FROM_SIZE $STEP_SIZE $TO_SIZE`; do
        echo -n "$ALGO ${SIZE}/${TO_SIZE} "
        TIME=$(${EXE} -r ${NREPS} -N ${SIZE} | grep -i "Intersection time" | egrep -o "[[:digit:]\.]+")
        echo "$SIZE $TIME" >> ${FNAME}
        echo "$TIME"
    done
    echo
done
