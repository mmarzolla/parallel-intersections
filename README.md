# Parallel Intersection-Counting algorithm

This repository contains a parallel implementation of the
intersection-counting algorithm described in the paper:

> M. Marzolla, G. Birolo, G. D'Angelo, P. Fariselli, "Parallel
> Intersection Counting on Shared-Memory Multiprocessors and GPUs"

## Requirements

The program has been compiled under Ubuntu Linux 18.04/20.04 with the
following dependencies:

- The [Thrust parallel algorithm library](https://thrust.github.io/).
  We used the most recent version from the Git repository

- [NVidia CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)

- GNU Make

## Installation

Clone the Thrust library into a local directory; the Makefile assumes
that Thrust is installed in `~/src/thrust`, but you can change this
by modifying the value of `THRUST_INCLUDE_PATH` in the Makefile.

    mkdir -p ~/src
    cd ~/src
    git clone --recursive https://github.com/NVIDIA/thrust.git

(you don't need to compile anything; Thrust is a headers-only library).

Fetch the input data into `~/src/intersections-data/` and unpack it:

    wget https://si-clusterraspberry.csr.unibo.it/downloads/intersections-data.tar.gz
    tar xvfz intersections-data.tar.gz

Clone this repository and build all executable:

    git clone https://github.com/mmarzolla/parallel-intersections
    cd parallel-intersections
    make all

The provided Makefile builds three executables:
`intersections_thrust_seq` (serial version),
`intersections_thrust_omp` (OpenMP version for CPU),
`intersections_thrust_cuda` (CUDA version for the GPU).

Do a quick test to see if everything works:

    make check

Test the sequential, OpenMP and CUDA intersection-counting algorithms
with the `Chr21` and `Exome` datasets:

    make check.med
    make check.big
