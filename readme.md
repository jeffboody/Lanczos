Lanczos Resampling
==================

Lanczos (pronounced Lanchos) resampling is similar to the
well known nearest neighbor and bilinear interpolation
techniques. Lanczos resampling method features improved
detail preservation and minimal generation of aliasing
artifacts. The main drawbacks of Lanczos resampling are
increased computation costs and the potential for "ringing"
artifacts which are most visible around sharp edges due to
the negative lobes in the interpolation kernel. Lanczos
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
kept in the interpolation function window. Note that the
support size may be referred to as the filter size or order
in other derivations. According to Jim Blinn, the Lanczos
(a = 3) kernel "keeps low frequencies and rejects high
frequencies better than any (achievable) filter we've seen
so far."

Lanczos Interpolation
---------------------

Lanczos interpolation of a one-dimensional input signal
s1(x) or two-dimensional input image s1(x,y) is calculated
as follows. Keep in mind that the input signal s1() is only
defined at discrete indices, however, it may also be viewed
as a continuous step function whose values are constant
between indices. The input signal s1() is zero outside of
the signal or image bounds. In contrast, the sampled signal
s2() is defined continuously.

	s1(x) = s1(floor(x)) : floor(x) = [0, n1]
	      = 0.0          : otherwise

	s2(x) = (1/w(x))*
	        SUM(i = -a + 1, i = a,
	            s1(floor(x) + i)*
	            L(i - x + floor(x),a))

	s1(x,y) = s1(floor(x), floor(y)) :
	          (floor(x), floor(y)) = ([0, w], [0,h))
	        = 0.0                    : otherwise

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
(j + 0.5) is the center of a sample in s2(). The step size
scales a point in s2() to a point in s1(). The final -0.5
term in x is a phase shift that causes the Lanczos
coefficient samples to be offset.

	step = n1/n2
	j    = [0..n2)
	x    = (j + 0.5)*step - 0.5

Upscaling and Downscaling
-------------------------

We often want to change from one sampling rate to another.
The process of representing a signal with more samples is
called interpolation or upsampling, whereas representing it
with less is called decimation or downsampling. When
upsampling, the equations above may be used without any
changes. However, when downsampling, an additional step must
be performed to adjust the filter scale (fs). This is
equivalent to applying a low pass filter to the input signal
to filter high frequency components. The Lanczos
interpolation equations may be adjusted for downscaling by
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
