## plot the speedup
##
## Run with:
##
## gnuplot plot_speedup.gp
##
## The plot goes to speedup.eps
##
## Last updated 2022-09-29 Moreno Marzolla

set term postscript eps color linewidth 1.2
set size .8
set output "speedup.eps"
set key top left
set xlabel "Number of processors (P)"
set ylabel "Speedup"
set title "Speedup (N = 50 x 10^6 intervals)"

T0_titan=system("cat test_results/titan_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")
T0_raptor=system("cat test_results/isi-raptor03_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")
T0_colossus=system("cat test_results/colossus_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")

plot [1:][1:] \
     "test_results/colossus_speedup.txt" using 1:(T0_colossus/$2) with l lw 2 lt 1 title "Machine A", \
     "test_results/isi-raptor03_speedup.txt" using 1:(T0_raptor/$2) with l lw 2 title "Machine B", \
     "test_results/titan_speedup.txt" using 1:(T0_titan/$2) with l lw 2 title "Machine C", \
     x with l lt 0 lw 1 notitle

