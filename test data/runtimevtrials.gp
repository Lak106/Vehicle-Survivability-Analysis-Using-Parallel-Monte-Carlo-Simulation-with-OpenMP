set terminal pngcairo size 1000,700
set output "runtime_vs_trials.png"
set title "Monte Carlo Survivability: Runtime vs Trials"
set xlabel "Number of Trials"
set ylabel "Elapsed Time seconds"
set grid
set key outside
set datafile separator ","
plot \
    "performance.csv" using 1:($2==1 ? $3 : 1/0) with linespoints linewidth 2 title "1 thread", \
    "performance.csv" using 1:($2==2 ? $3 : 1/0) with linespoints linewidth 2 title "2 threads", \
    "performance.csv" using 1:($2==4 ? $3 : 1/0) with linespoints linewidth 2 title "4 threads", \
    "performance.csv" using 1:($2==8 ? $3 : 1/0) with linespoints linewidth 2 title "8 threads"