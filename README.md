# Parallel Intersection-Counting algorithm

This repository contains the implementation of the parallel
intersection-counting algorithm described in the paper:

> M. Marzolla, G. Birolo, G. D'Angelo, P. Fariselli, "Parallel
> Intersection Counting on Shared-Memory Multiprocessors and GPUs",
> Future Generation Computer Systems volume 159, october 2024,
> pp. 423-431, ISSN 0167-739X. DOI
> [j.future.2024.05.039](https://doi.org/10.1016/j.future.2024.05.039)

## Requirements

The program has been compiled under Ubuntu Linux 18.04/20.04/22.04
with the following dependencies:

- The [Thrust parallel algorithm library](https://thrust.github.io/).
  We used the most recent version from the Git repository

- [NVidia CUDA Toolkit](https://developer.nvidia.com/cuda-downloads);
  you should install the version from NVidia web site rather than the
  version included in Ubuntu's software repository, since the latter
  might be incompatible with Thrust.

- The Parallel STL library (part of the GNU C Compiler). GNU STL
  requires Intel's [OneAPI Threading Building Blocks
  (oneTBB)](https://github.com/oneapi-src/oneTBB) library, which may
  or may not be already installed with the GCC compiler. If not,
  you can install it with `sudo apt install libtbb-dev`

- `libhts` (install with `sudo apt install libhts-dev`)

- GNU Make

## Installation

### Step 1

Clone the [Thrust library](https://thrust.github.io/) into a local
directory:

    mkdir -p ~/src
    cd ~/src
    git clone --recursive https://github.com/NVIDIA/thrust.git

You don't need to compile anything: Thrust is a headers-only library.

The present repository assumes that Thrust is installed in
`~/src/thrust`, but you can change this by modifying the value of
`THRUST_INCLUDE_PATH` in the Makefile.

### Step 2 (optional)

To replicate the tests described in the paper, you need to fetch the
input data into `~/src/intersections-data/` and unpack them using the
following commands:

    wget https://si-clusterraspberry.csr.unibo.it/downloads/intersections-data.tar.gz
    tar xvfz intersections-data.tar.gz

**WARNING** the archive is very big (more than 16GB), and becomes even
bigger (about 64GB) once decompressed. Please make sure that you have
enough disk space.

### Step 3

Clone this repository and build all executables:

    git clone https://github.com/mmarzolla/parallel-intersections
    cd parallel-intersections
    make all

Four programs should be built: `intersections_thrust_seq` (serial
version), `intersections_thrust_omp` (OpenMP version for CPU),
`intersections_thrust_cuda` (CUDA version for the GPU) and
`intersections_stl` (parallel STL version for the CPU).

### Step 4

Perform a quick check to see if everything works:

    make check

### Step 5 (optional)

Test all implementations with the `Chr21` and `Exome` datasets from
step 2 above:

    make test.med
    make test.big

## Known issues

There are [issues](https://github.com/NVIDIA/nccl/issues/102) with the
NVidia C compiler bundled with Ubuntu 22.04: the serial and OpenMP
versions of the parallel intersection-counting program compile and run
fine, but the CUDA version does not compile due to the following
error:

```
/usr/include/c++/11/bits/std_function.h:435:145: error: parameter packs not expanded with ‘...’:
  435 |         function(_Functor&& __f)
      |                                                                                                                                                 ^
/usr/include/c++/11/bits/std_function.h:435:145: note:         ‘_ArgTypes’
/usr/include/c++/11/bits/std_function.h:530:146: error: parameter packs not expanded with ‘...’:
  530 |         operator=(_Functor&& __f)
      |                                                                                                                                                  ^
/usr/include/c++/11/bits/std_function.h:530:146: note:         ‘_ArgTypes’
make: *** [Makefile:112: thrust_count_cuda.o] Error 1
```

To solve this problem you need to install the NVidia CUDA Toolkit from
[NVidia web site](https://developer.nvidia.com/cuda-downloads), rather
than the version provided with Ubuntu.

If you encounter this error message:

```
/home/marzolla/src/thrust/thrust/system/cuda/config.h:116:2: error: #error The version of CUB in your include path is not compatible with this release of Thrust. CUB is now included in the CUDA Toolkit, so you no longer need to use your own checkout of CUB. Define THRUST_IGNORE_CUB_VERSION_CHECK to ignore this.
 #error The version of CUB in your include path is not compatible with this release of Thrust. CUB is now included in the CUDA Toolkit, so you no longer need to use your own checkout of CUB. Define THRUST_IGNORE_CUB_VERSION_CHECK to ignore this.
  ^~~~~
```

try compiling with the following command at Step 3 above:

    NVCFLAGS=-DTHRUST_IGNORE_CUB_VERSION_CHECK make all
