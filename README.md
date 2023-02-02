# Parallel Intersection-Counting algorithm

This repository contains a parallel implementation of the
intersection-counting algorithm described in the paper:

> M. Marzolla, G. Birolo, G. D'Angelo, P. Fariselli, "Parallel
> Intersection Counting on Shared-Memory Multiprocessors and GPUs"

## Requirements

The program has been compiled under Ubuntu Linux 18.04/20.04 with the
following dependencies:

- The [Thrust parallel algorithm library](https://thrust.github.io/).
  We used the most recent version from the Git repository; simply
  checkout a local copy of the Thrust repository: there is no need to
  build Thrust, since it is include-only.

- [NVidia CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)

- GNU Make

## Building

The enclosed Makefile builds three executables:
`intersections_thrust_seq` (serial version),
`intersections_thrust_omp` (OpenMP version for CPU),
`intersections_thrust_cuda` (CUDA version for the GPU).

Simply issue the command:

    make all

to build all executables; type `make` to see the full list of targets
supported by the Makefile.



