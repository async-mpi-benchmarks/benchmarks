set xlabel 'Amount of iteration'
set ylabel 'Tics measured'



set term png size 4000,4000
set output "plot_time_xeon_phi.png"
set grid 
plot 'time_xeon_phi.txt' using 1:2 title "rdtsc" pointtype 4 pointsize 3.0 lt rgb "blue", 'time_xeon_phi.txt' using 1:5  title "sync-rdtscp" pointtype 12 pointsize 3.0 lt rgb "red"
