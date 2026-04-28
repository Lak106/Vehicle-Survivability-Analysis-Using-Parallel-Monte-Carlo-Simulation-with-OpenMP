set terminal pngcairo size 1200,700
set output "scenario_probabilities.png"
set title "Scenario Comparison: Outcome Probabilities"
set xlabel "Test Scenario"
set ylabel "Probability"
set grid
set datafile separator ","
set style data histograms
set style histogram clustered gap 1
set style fill solid border -1
set boxwidth 0.9
set yrange [0:1]
set key outside
plot \
    "test_comparison.csv" using 5:xtic(1) title "No Penetration", \
    "test_comparison.csv" using 6 title "Penetration", \
    "test_comparison.csv" using 7 title "Catastrophic Kill", \
    "test_comparison.csv" using 8 title "Survivability"