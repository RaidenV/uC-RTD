#!/user/local/bin/gnuplot -persist
set xlabel "Time (seconds)"
set ylabel "Angle (degrees)"
set label 1 "Kp: 0" at .85, 45 tc rgb "blue"
set label 2 "Ki: 0" at .85, 35 tc rgb "blue"
set label 3 "Kd: 0" at .85, 25 tc rgb "blue"
set label 4 "Kp: 1" at .85, 15 tc rgb "red"
set label 5 "Ki: 0" at .85, 5 tc rgb "red"
set label 6 "Kd: 0" at .85, -5 tc rgb "red"
set xrange [0:1.2]
set yrange [-10:101]
set title "Pid Loop plots: Plot 18.48.15 11-17-2015 vs. Plot 18.47.54 11-17-2015"
plot '/home/raidenv/PIDPlot/Plots/Plot 18.48.15 11-17-2015.dat' using 1:2 with line lw 2 lt rgb "blue" notitle,\
'/home/raidenv/PIDPlot/Plots/Plot 18.47.54 11-17-2015.dat' using 1:2 with line lw 2 lt rgb "red" notitle
pause -1