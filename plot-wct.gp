## plot the wall-clock time using data in test_results/
##
## Run with:
##
## gnuplot plot-wct.gp
##
## The plot goes to wct.eps
##
## Last updated 2024-02-01 Moreno Marzolla

set term postscript eps color linewidth 1.2
set size .7
set output "wct.eps"
set logscale y
set key bottom right maxrows 3
set xlabel "Number of intervals (N)"
set ylabel "Wall-clock time (s)"
set title "Wall-clock time"

wctime(n, t) = t

plot [1e6:] "test_results/colossus_cuda_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 1 pt 1 notitle, \
     "test_results/isi-raptor03_cuda_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 1 pt 2 notitle, \
     "test_results/titan_cuda_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 1 pt 3 notitle, \
     "test_results/colossus_omp_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 2 pt 1 notitle, \
     "test_results/isi-raptor03_omp_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 2 pt 2 notitle, \
     "test_results/titan_omp_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 2 pt 3 notitle, \
     "test_results/colossus_seq_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 3 pt 1 notitle, \
     "test_results/isi-raptor03_seq_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 3 pt 2 notitle, \
     "test_results/titan_seq_wct.txt"  using 1:(wctime($1,$2)) with lp lw 2 lt 3 pt 3 notitle, \
  [-10:-9] x with l lw 2 lt 1 title "CUDA", \
  [-10:-9] x with l lw 2 lt 2 title "OpenMP", \
  [-10:-9] x with l lw 2 lt 3 title "Serial", \
  [-10:-9] x with p lt 1 pt 1 title "Machine A", \
  [-10:-9] x with p lt 1 pt 2 title "Machine B", \
  [-10:-9] x with p lt 1 pt 3 title "Machine C"
