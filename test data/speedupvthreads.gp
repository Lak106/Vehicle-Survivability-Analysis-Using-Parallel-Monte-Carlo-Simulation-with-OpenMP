set terminal pngcairo size 1000,700
set output "speedup_vs_threads.png"
set title "Monte Carlo Survivability: Speedup vs Threads"
set xlabel "Threads"
set ylabel "Speedup"
set grid
set key outside
set datafile separator ","
set xtics 1,1,8
plot \
    "speedup.csv" using ($1==100000 ? $2 : 1/0):($1==100000 ? $4 : 1/0) with linespoints linewidth 2 title "100000 trials", \
    "speedup.csv" using ($1==500000 ? $2 : 1/0):($1==500000 ? $4 : 1/0) with linespoints linewidth 2 title "500000 trials", \
    "speedup.csv" using ($1==1000000 ? $2 : 1/0):($1==1000000 ? $4 : 1/0) with linespoints linewidth 2 title "1000000 trials", \
    "speedup.csv" using ($1==5000000 ? $2 : 1/0):($1==5000000 ? $4 : 1/0) with linespoints linewidth 2 title "5000000 trials"