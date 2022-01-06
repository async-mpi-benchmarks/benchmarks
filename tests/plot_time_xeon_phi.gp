set xlabel 'Amount of iteration'
set ylabel 'Tics measured'


set yrange [0:6000]
set term png size 1000,1000
set output "plot_time_xeon_phi.png"
set grid 
plot 'time_xeon_phi.txt' using 1:2 title "rdtsc" , 'time_xeon_phi.txt' using 1:5  title "sync-rdtscpba", 'time_xeon_phi.txt' using 1:8  title "shifted rdtscp" , 'time_xeon_phi.txt' using 1:11  title "fenced rdtscp"
