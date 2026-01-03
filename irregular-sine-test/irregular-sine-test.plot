# gnuplot
# load "irregular-sine-test.plot"

# style
set style line 1 lt rgb "#FF0000" lw 3 pt 6
set style line 2 lt rgb "#00FF00" lw 3 pt 6

# plot data
plot "irregular-sine-3-32.dat" using 1:2 with points ls 1 title 'Original (32)', \
     "irregular-sine-3-32-16.dat" using 1:2 with linespoints ls 2 title 'Resampled (16)'
