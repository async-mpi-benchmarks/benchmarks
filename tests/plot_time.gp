set xlabel 'Amount of iteration'
set ylabel 'Tics measured'


set yrange [0:4000]
set term png size 1000,1000
set output "plot_time.png"
set grid 
plot 'time.txt' using 1:2 title "rdtsc" , 'time.txt' using 1:5  title "sync-rdtscp"
