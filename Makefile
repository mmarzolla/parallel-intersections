##############################################################################
##
## Configuration parameters (edit according to your environment)
##
##############################################################################

THRUST_INCLUDE_PATH := ${HOME}/src/thrust

CUDA_LIB_PATH := /usr/local/cuda/lib64

# where are the (big) .bam and .bed data stored (only if you need to
# test the program)
DATA_PATH := ${HOME}/intersections-data

BITS_COUNT_PER_INTERVAL := ${HOME}/src/bits/bin/bits_count_per_interval_cuda

BITS_COUNT := ${HOME}/src/bits/bin/bits_count_cuda

##############################################################################
##
## End Configuration (you should not need to modify anything below)
##
##############################################################################

CXXFLAGS+=-Wall -std=c++14 -pedantic -O2 -fopenmp -I${THRUST_INCLUDE_PATH} -I${THRUST_INCLUDE_PATH}/dependencies/libcudacxx/include
CPPFLAGS+=
LDFLAGS+=-fopenmp
LDLIBS+=-L${CUDA_LIB_PATH} -lm -lrt -lhts -lcudart
NVCC:=nvcc
NVCFLAGS+=-x cu -O2 -I${THRUST_INCLUDE_PATH} -I${THRUST_INCLUDE_PATH}/dependencies/libcudacxx/include -I${THRUST_INCLUDE_PATH}/dependencies/cub

# name of the executable
EXE:=intersections

# Thrust-based OpenMPversion
EXE_OMP:=${EXE}_thrust_omp

# Thrust-based OpenMP with parallel sort from the Standard Template Library
EXE_OMP_STL:=${EXE}_thrust_omp_stl

# Sequential version
EXE_SEQ:=${EXE}_thrust_seq

# CUDA-based version
EXE_CUDA:=${EXE}_thrust_cuda

EXES:=$(EXE_OMP) $(EXE_SEQ) $(EXE_CUDA) # $(EXE_OMP_STL)

ALL: read_bam $(EXES)

tests: ${EXES}
	./test_wct.sh
	./test_speedup.sh

check: read_bam $(EXES)
	./$(EXE_SEQ) -m panel_01.bam -d target.bed
	./$(EXE_OMP) -m panel_01.bam -d target.bed
	./$(EXE_CUDA) -m panel_01.bam -d target.bed

test.med: $(EXES)
	for ALGO in $(EXES); do \
		./$${ALGO} -m ${DATA_PATH}/HG00258.chrom20.ILLUMINA.bwa.GBR.exome.20120522.bam -d ${DATA_PATH}/hsa37-cds-chr20.bed ; \
	done

test.big: $(EXES)
	for ALGO in $(EXES); do \
		./$${ALGO} -m ${DATA_PATH}/HG00258.mapped.ILLUMINA.bwa.GBR.exome.20120522.bam -d ${DATA_PATH}/hsa37-cds.bed ; \
	done

test.bits: $(EXE_CUDA) $(BITS_COUNT_PER_INTERVAL) $(BITS_COUNT)
	@echo "**"
	@echo "** ${EXE_CUDA}"
	@echo "**"
	./$(EXE_CUDA) -m ${DATA_PATH}/HG00258.chrom20.ILLUMINA.bwa.GBR.exome.20120522.bam -d ${DATA_PATH}/hsa37-cds-split.bed
	@echo "**"
	@echo "** ${BITS_COUNT_PER_INTERVAL}"
	@echo "**"
	$(BITS_COUNT_PER_INTERVAL) -a ${DATA_PATH}/HG00258.chrom20.ILLUMINA.bwa.GBR.exome.20120522.bed -b ${DATA_PATH}/hsa37-cds-split.bed -g ${DATA_PATH}/hsa37.genome
	@echo "**"
	@echo "** ${BITS_COUNT}"
	@echo "**"
	$(BITS_COUNT) -a ${DATA_PATH}/HG00258.chrom20.ILLUMINA.bwa.GBR.exome.20120522.bed -b ${DATA_PATH}/hsa37-cds-split.bed -g ${DATA_PATH}/hsa37.genome

read_bam: read_bam.cpp
	$(CXX) -o read_bam read_bam.cpp -lhts

$(EXE_OMP): main.o interval.o thrust_count_omp.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_OMP_STL): main.o interval.o thrust_count_omp_stl.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_SEQ): main.o interval.o thrust_count_seq.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_CUDA): main.o interval.o thrust_count_cuda.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

thrust_count_omp.o: CPPFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP # -D_GLIBCXX_PARALLEL
thrust_count_omp.o: thrust_count.cc thrust_count.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

thrust_count_omp_stl.o: CPPFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP -DUSE_STL_SORT # -D_GLIBCXX_PARALLEL
thrust_count_omp_stl.o: thrust_count.cc thrust_count.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

thrust_count_seq.o: CPPFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP
thrust_count_seq.o: thrust_count.cc thrust_count.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

thrust_count_cuda.o: NVCFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CUDA
thrust_count_cuda.o: thrust_count.cc thrust_count.hh
	$(NVCC) $(NVCFLAGS) -c $< -o $@

figures: plot_speedup.gp plot_wct.gp
	gnuplot plot_speedup.gp
	gnuplot plot_wct.gp

clean:
	\rm -f read_bam *.o $(EXES)

distclean: clean
