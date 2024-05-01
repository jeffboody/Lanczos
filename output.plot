# gnuplot
# load "output.plot"

# style
set style line 1 lt rgb "#FF0000" lw 3 pt 6
set style line 2 lt rgb "#00FF00" lw 3 pt 6
set style line 3 lt rgb "#0000FF" lw 3 pt 6

# plot data
plot "s1.dat" using 1:2 with linespoints ls 3 title 'Original', \
     "s2d.dat" using 1:2 with linespoints ls 1 title 'Downsampled', \
     "s2u.dat" using 1:2 with linespoints ls 2 title 'Upsampled'
