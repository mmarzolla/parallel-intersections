## plot the speedup
##
## Run with:
##
## gnuplot plot-speedup.gp
##
## The plot goes to speedup.eps
##
## Last updated 2024-02-01 Moreno Marzolla

set term postscript eps color linewidth 1.2
set size .7
set output "speedup.eps"
set key top right
set xlabel "Number of processors (P)"
set ylabel "Speedup"
set title "Scalability (N = 50 x 10^6 intervals)"

T0_titan=system("cat 2024-02-01-test-results/titan_omp_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")
T0_raptor=system("cat 2024-02-01-test-results/isi-raptor03_omp_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")
T0_colossus=system("cat 2024-02-01-test-results/colossus_speedup.txt | grep -v '^#' | awk 'NR==1 {print $2}'")

plot [1:][1:6] \
     "2024-02-01-test-results/colossus_speedup.txt" using 1:(T0_colossus/$2) with l lw 3 lt 1 title "Machine A", \
     "2024-02-01-test-results/isi-raptor03_omp_speedup.txt" using 1:(T0_raptor/$2) with l lw 3 title "Machine B", \
     "2024-02-01-test-results/titan_omp_speedup.txt" using 1:(T0_titan/$2) with l lw 3 title "Machine C", \
     x with l lt 0 lw 2 notitle

