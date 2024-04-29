# gnuplot
# load "lanczos3.plot"

# style
set style line 1 lt rgb "#FF0000" lw 3 pt 6

# Lanczos kernel
L(x,a) = sin(pi*x)*sin(pi*x/a)/((pi*x)*(pi*x/a))

# plot the Lanczos3 kernel
set xrange [-3.0:3.0]
plot L(x,3) with linespoints ls 1 title 'Lanczos3 Kernel'
