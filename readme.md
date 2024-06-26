Lanczos Resampling
==================

Lanczos (pronounced Lanchos) resampling is similar to the
well known nearest neighbor and bilinear interpolation
techniques. Lanczos resampling features improved detail
preservation and minimal generation of aliasing artifacts.
The main drawbacks of Lanczos resampling are increased
computation costs and the potential for "ringing" artifacts
which are most visible around sharp edges due to the
negative lobes in the interpolation kernel. Lanczos
resampling may be used to perform operations such as scaling
and rotation by resampling the input signal.

Lanczos Kernel
--------------

The Lanczos kernel is defined for a given window.

	L(x,a) = sinc(x)*sinc(x/a) : -a <= x < a
	       = 0.0               : otherwise

Where the normalized sinc function is given by.

	sinc(x) = 1.0              : x = 0.0
	        = sin(PI*x)/(PI*x) : otherwise

![Lanczos3 Kernel](lanczos3.jpg?raw=true "Lanczos3 Kernel")

The support size (a) corresponds to the number of lobes
kept in the interpolation function window. According to Jim
Blinn, the Lanczos (a = 3) kernel "keeps low frequencies and
rejects high frequencies better than any (achievable) filter
we've seen so far."

Lanczos Interpolation
---------------------

Lanczos interpolation may be performed on a one-dimensional
input signal s1(x) or two-dimensional input image s1(x,y)
The term signal will henceforth apply to images. Keep in
mind that the input signal s1() is only defined at discrete
indices, however, it may also be viewed as a continuous step
function whose values are constant between indices. The
Lanczos interpolation causes the input signal s1() to be
sampled beyond its bounds. In this case, samples outside the
bounds may be zero or clamped to the edge of s1(). The
notation s1() indicates that the input signal is accessed
continuously while s1[] indicates that the input signal is
accessed by index.

	# zero outside bounds
	s1(x) = s1[floor(x)] :
	        x = [0, n1)
	      = 0.0 : otherwise

	s1(x,y) = s1[floor(x), floor(y)] :
	          x = [0, w1), y = [0,h1)
	        = 0.0 : otherwise

	# clamp-to-edge outside bounds
	s1(x) = s1[clamp(floor(x), 0, n1 - 1)]

	s1(x,y) = s1[clamp(floor(x), 0, w1 - 1),
	             clamp(floor(y), 0, h1 - 1)]

In contrast, the sampled signal s2() is defined continuously
and may be calculated as follows.

	s2(x) = (1/w(x))*
	        SUM(i = -a + 1, i = a,
	            s1(floor(x) + i)*
	            L(i - x + floor(x),a))

	s2(x,y) = (1/w(x,y))*
	          SUM(i = -a + 1, i = a,
	              SUM(j = -a + 1, j = a,
	                  s1(floor(x) + j, floor(y) + i)*
	                  L(j - x + floor(x),a)*
	                  L(i - y + floor(y),a)))

The filter weight (w) is required to preserve flux (the
partition of unity property) as the sum is only an
approximation of the infinite Lanczos kernel.

	w(x) = SUM(i = -a + 1, i = a,
	           L(i - x + floor(x),a))

	w(x,y) = SUM(i = -a + 1, i = a,
	             SUM(j = -a + 1, j = a,
	                 L(j - x + floor(x),a)*
	                 L(i - y + floor(y),a)))

The range of s2() may be greater than that of s1() due to
the lobes of L(). For example, an input signal s1()
corresponding to pixel colors in the range of [0.0,1.0]
need to be clamped to the same range to ensure that the
output values do not overflow when converted back to
unsigned bytes of [0,255].

When resampling a signal from n1 samples to n2 samples the
following sample positions are used. The index j is used to
represent the samples from the sampled signal s2(). The term
(j + 0.5) is the center of a sample in s2(). The step scales
a point in s2() to a point in s1(). The final -0.5 term in x
is a phase shift that causes the Lanczos coefficient samples
to be offset.

	step = n1/n2
	j    = [0..n2)
	x    = (j + 0.5)*step - 0.5

Upsampling and Downsampling
---------------------------

We often want to change from one sampling rate to another.
The process of representing a signal with more samples is
called interpolation or upsampling, whereas representing it
with less is called decimation or downsampling. When
upsampling, the equations above may be used without any
changes. However, when downsampling, an additional step must
be performed to adjust the filter scale (fs). This is
equivalent to applying a low pass filter to the input signal
to filter high frequency components. The Lanczos
interpolation equations may be adjusted for downsampling by
an integer factor as follows.

	fs = n1/n2

	s2(x) = (1/w(x))*
	        SUM(i = -(fs*a) + 1, i = (fs*a),
	            s1(floor(x) + i)*
	            L((i - x + floor(x))/fs,a))

	s2(x,y) = (1/w(x,y))*
	          SUM(i = -(fs*a) + 1, i = (fs*a),
	              SUM(j = -(fs*a) + 1, j = (fs*a),
	                  s1(floor(x) + j, floor(y) + i)*
	                  L((j - x + floor(x))/fs,a)*
	                  L((i - y + floor(y))/fs,a)))

When upsampling by a power-of-two, the Lanczos kernel cycles
between 2^level sets of values. For example, consider the
first 4 outputs of the lanzcos-test upsample example. Notice
that the outputs for L() repeat for j={0,2} and j={1,3}.

	upsample n1=10, n2=20
	j=0, x=-0.250000
	i=-2, L(-2.750000)=0.007356, S1(-3.000000)=0.100000
	i=-1, L(-1.750000)=-0.067791, S1(-2.000000)=0.100000
	i=0, L(-0.750000)=0.270190, S1(-1.000000)=0.100000
	i=1, L(0.250000)=0.890067, S1(0.000000)=0.100000
	i=2, L(1.250000)=-0.132871, S1(1.000000)=0.300000
	i=3, L(2.250000)=0.030021, S1(2.000000)=0.400000
	s2=0.082129, s2/w=0.082379, w=0.996972
	j=1, x=0.250000
	i=-2, L(-2.250000)=0.030021, S1(-2.000000)=0.100000
	i=-1, L(-1.250000)=-0.132871, S1(-1.000000)=0.100000
	i=0, L(-0.250000)=0.890067, S1(0.000000)=0.100000
	i=1, L(0.750000)=0.270190, S1(1.000000)=0.300000
	i=2, L(1.750000)=-0.067791, S1(2.000000)=0.400000
	i=3, L(2.750000)=0.007356, S1(3.000000)=0.300000
	s2=0.134869, s2/w=0.135279, w=0.996972
	j=2, x=0.750000
	i=-2, L(-2.750000)=0.007356, S1(-2.000000)=0.100000
	i=-1, L(-1.750000)=-0.067791, S1(-1.000000)=0.100000
	i=0, L(-0.750000)=0.270190, S1(0.000000)=0.100000
	i=1, L(0.250000)=0.890067, S1(1.000000)=0.300000
	i=2, L(1.250000)=-0.132871, S1(2.000000)=0.400000
	i=3, L(2.250000)=0.030021, S1(3.000000)=0.300000
	s2=0.243853, s2/w=0.244594, w=0.996972
	j=3, x=1.250000
	i=-2, L(-2.250000)=0.030021, S1(-1.000000)=0.100000
	i=-1, L(-1.250000)=-0.132871, S1(0.000000)=0.100000
	i=0, L(-0.250000)=0.890067, S1(1.000000)=0.300000
	i=1, L(0.750000)=0.270190, S1(2.000000)=0.400000
	i=2, L(1.750000)=-0.067791, S1(3.000000)=0.300000
	i=3, L(2.750000)=0.007356, S1(4.000000)=0.200000
	s2=0.345945, s2/w=0.346996, w=0.996972

When downsampling by a power-of-two, the Lanczos kernel
becomes independent of x since (x - floor(x)) becomes a
constant which matches the phase shift. For example,
consider the first 2 outputs of the lanzcos-test
downsample example. Notice that the outputs of L()
repeat for each j.

	downsample n1=10, n2=5
	j=0, x=0.500000
	i=-5, L(-2.750000)=0.007356, S1(-5.000000)=0.100000
	i=-4, L(-2.250000)=0.030021, S1(-4.000000)=0.100000
	i=-3, L(-1.750000)=-0.067791, S1(-3.000000)=0.100000
	i=-2, L(-1.250000)=-0.132871, S1(-2.000000)=0.100000
	i=-1, L(-0.750000)=0.270190, S1(-1.000000)=0.100000
	i=0, L(-0.250000)=0.890067, S1(0.000000)=0.100000
	i=1, L(0.250000)=0.890067, S1(1.000000)=0.300000
	i=2, L(0.750000)=0.270190, S1(2.000000)=0.400000
	i=3, L(1.250000)=-0.132871, S1(3.000000)=0.300000
	i=4, L(1.750000)=-0.067791, S1(4.000000)=0.200000
	i=5, L(2.250000)=0.030021, S1(5.000000)=0.400000
	i=6, L(2.750000)=0.007356, S1(6.000000)=0.600000
	s2=0.437796, s2/w=0.219563, w=1.993943
	j=1, x=2.500000
	i=-5, L(-2.750000)=0.007356, S1(-3.000000)=0.100000
	i=-4, L(-2.250000)=0.030021, S1(-2.000000)=0.100000
	i=-3, L(-1.750000)=-0.067791, S1(-1.000000)=0.100000
	i=-2, L(-1.250000)=-0.132871, S1(0.000000)=0.100000
	i=-1, L(-0.750000)=0.270190, S1(1.000000)=0.300000
	i=0, L(-0.250000)=0.890067, S1(2.000000)=0.400000
	i=1, L(0.250000)=0.890067, S1(3.000000)=0.300000
	i=2, L(0.750000)=0.270190, S1(4.000000)=0.200000
	i=3, L(1.250000)=-0.132871, S1(5.000000)=0.400000
	i=4, L(1.750000)=-0.067791, S1(6.000000)=0.600000
	i=5, L(2.250000)=0.030021, S1(7.000000)=0.800000
	i=6, L(2.750000)=0.007356, S1(8.000000)=0.900000
	s2=0.678627, s2/w=0.340344, w=1.993943

As a result, the Lanczos kernel may be precomputed for these
cases to eliminate the expensive sinc function computation
cost.

Separability
------------

The Lanczos kernel is nonseparable, however, in practice
implementations may choose to perform separate passes in
order to improve performance at a small reduction to
quality. Similarly, recursively resampling a signal multiple
times (e.g. for texture mipmap LODs) reduces the quality
compared to resizing the original signal to multiple scales.
Both options should be considered depending on the quality
and performance requirements.

Example
-------

Run the lancos-test example and use gnuplot to see how
Lanczos resampling affects a one-dimensional signal.

	make
	./lanczos-test
	gnuplot
	> load "output.plot"

![Example](output.jpg?raw=true "Example")

References
----------

* [Lanczos Resampling](https://en.wikipedia.org/wiki/Lanczos_resampling)
* [Downsampling](https://en.wikipedia.org/wiki/Downsampling_(signal_processing))
* [Upsampling](https://en.wikipedia.org/wiki/Upsampling)
* [Lanczos interpolation and resampling](https://www.youtube.com/watch?v=ijmd6XyG2HA)
* [Interpolation Algorithms](https://pixinsight.com/doc/docs/InterpolationAlgorithms/InterpolationAlgorithms.html)
* [Filters for Common Resampling Tasks](http://www.realitypixels.com/turk/computergraphics/ResamplingFilters.pdf)
* [Pillow Implementation](https://github.com/python-pillow/Pillow/blob/main/src/libImaging/Resample.c)
* [imageresampler](https://github.com/richgel999/imageresampler)

License
=======

This code was implemented by
[Jeff Boody](mailto:jeffboody@gmail.com)
under The MIT License.

	Copyright (c) 2024 Jeff Boody

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
