# gnuplot
# load "sine-test.plot"

# style
set style line 1 lt rgb "#FF0000" lw 3 pt 6
set style line 2 lt rgb "#00FF00" lw 3 pt 6
set style line 3 lt rgb "#0000FF" lw 3 pt 6
set style line 4 lt rgb "#FF00FF" lw 3 pt 6
set style line 5 lt rgb "#FFFF00" lw 3 pt 6

# plot data
plot "sine-3-16.dat" using 1:2 with linespoints ls 3 title 'Original (16)', \
     "sine-3-16-8.dat" using 1:2 with linespoints ls 1 title 'Fast Downsampled (8)', \
     "sine-3-16-12.dat" using 1:2 with linespoints ls 4 title 'Slow Downsampled (12)', \
     "sine-3-16-32.dat" using 1:2 with linespoints ls 2 title 'Fast Upsampled (32)', \
     "sine-3-16-24.dat" using 1:2 with linespoints ls 5 title 'Slow Upsampled (24)'
