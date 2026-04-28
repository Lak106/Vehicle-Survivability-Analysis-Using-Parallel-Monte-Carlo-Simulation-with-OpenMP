set terminal pngcairo size 1000,700
set output "scenario_survivability.png"
set title "Scenario Comparison: Survivability Probability"
set xlabel "Test Scenario"
set ylabel "Survivability Probability"
set grid
set datafile separator ","
set style data histograms
set style fill solid border -1
set boxwidth 0.8
set yrange [0:1]
plot "test_comparison.csv" using 7:xtic(1) title "Survivability"