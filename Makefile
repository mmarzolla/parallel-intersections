##############################################################################
##
## Configuration parameters (edit according to your environment)
##
##############################################################################

THRUST_INCLUDE_PATH := ${HOME}/src/thrust

CUDA_LIB_PATH := /usr/local/cuda/lib64

# location where the (big) .bam and .bed files are stored (only if you
# need to test the program using the same data files from the paper)
DATA_PATH := ${HOME}/src/intersections-data

##############################################################################
##
## End Configuration (you should not need to modify anything below)
##
##############################################################################

CXXFLAGS+=-Wall -std=c++14 -pedantic -O2 -fopenmp -I${THRUST_INCLUDE_PATH} -I${THRUST_INCLUDE_PATH}/dependencies/libcudacxx/include
CPPFLAGS+=
LDFLAGS+=-fopenmp
LDLIBS+=-lm -lrt -lhts
NVCC?=nvcc
NVCFLAGS+=-x cu -O2 -I${THRUST_INCLUDE_PATH}

# name of the executable
EXE:=intersections

# Thrust-based OpenMPversion
EXE_OMP:=${EXE}_omp

# Sequential version
EXE_SEQ:=${EXE}_seq

# CUDA-based version
EXE_CUDA:=${EXE}_cuda

# STL-based version
EXE_STL:=${EXE}_stl

EXES:=$(EXE_SEQ) $(EXE_OMP) $(EXE_STL) $(EXE_CUDA)

# Use the C++ compiler instead of C to link object files
LINK.o = $(LINK.cc)

help:
	@echo
	@echo "Available targets:"
	@echo
	@echo "all        build all executables"
	@echo "serial     build the sequential program only"
	@echo "omp        build the OpenMP program only"
	@echo "stl        build the STL program only"
	@echo "cuda       build the CUDA program only"
	@echo "clean      remove temporary build files"
	@echo "distclean  remove temporary files"
	@echo "check      quick test"
	@echo "tests      run comprehensive performance tests (requires full dataset)"
	@echo "test.med   test with the \"medium\" dataset"
	@echo "test.big   test with the \"big\" dataset"
	@echo

all: read_bam $(EXES)

serial: $(EXE_SEQ)

omp: $(EXE_OMP)

cuda: $(EXE_CUDA)

stl: $(EXE_STL)

tests: ${EXES}
	./test_wct.sh
	./test_speedup.sh

check: read_bam $(EXES)
	for ALGO in $(EXES); do \
		./$${ALGO} -m panel_01.bam -d target.bed ; \
	done

test.med: $(EXES)
	for ALGO in $(EXES); do \
		./$${ALGO} -r 5 -m ${DATA_PATH}/HG00258.chrom20.ILLUMINA.bwa.GBR.exome.20120522.bam -d ${DATA_PATH}/hsa37-cds-chr20-split.bed ; \
	done

test.big: $(EXES)
	for ALGO in $(EXES); do \
		./$${ALGO} -r 5 -m ${DATA_PATH}/HG00258.mapped.ILLUMINA.bwa.GBR.exome.20120522.bam -d ${DATA_PATH}/hsa37-cds-split.bed ; \
	done

read_bam: read_bam.cpp
	$(CXX) -o read_bam read_bam.cpp -lhts

$(EXE_OMP): main.o interval.o thrust_count_omp.o utils.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_SEQ): main.o interval.o thrust_count_seq.o utils.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_STL): LDLIBS+=-ltbb
$(EXE_STL): main.o interval.o stl_count_omp.o utils.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXE_CUDA): LDLIBS+=-lcudart
$(EXE_CUDA): LDFLAGS+=-L/usr/local/cuda/lib64
$(EXE_CUDA): main.o interval.o thrust_count_cuda.o utils.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

thrust_count_omp.o: CPPFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP # -D_GLIBCXX_PARALLEL
thrust_count_omp.o: thrust_count.cc count_intersections.hh utils.hh interval.hh endpoint.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

thrust_count_seq.o: CPPFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP
thrust_count_seq.o: thrust_count.cc count_intersections.hh utils.hh interval.hh endpoint.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

thrust_count_cuda.o: NVCFLAGS+=-DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CUDA
thrust_count_cuda.o: thrust_count.cc count_intersections.hh utils.hh interval.hh endpoint.hh
	$(NVCC) $(NVCFLAGS) -c $< -o $@

stl_count_omp.o: CXXFLAGS=-std=c++17 -O2 -Wall -Wpedantic -fopenmp
stl_count_omp.o: stl_count.cc count_intersections.hh utils.hh interval.hh endpoint.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

figures: plot-speedup.gp plot-wct.gp
	gnuplot plot-speedup.gp
	gnuplot plot-wct.gp

clean:
	\rm -f read_bam *.o $(EXES)

distclean: clean
