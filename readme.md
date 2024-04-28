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

The Lanczos kernel is defined for a given window.

	L(x,a) = sinc(x)*sinc(x/a) : -a <= x < a
	       = 0.0               : otherwise

The normalized sinc function is given by.

	sinc(x) = 1.0              : x = 0.0
	        = sin(PI*x)/(PI*x) : otherwise

The support size (a) corresponds to the number of lobes
kept in the interpolation function window. Note that the
support size may be referred to as the filter size or order
in other derivations. According to Jim Blinn, the Lanczos
(a = 3) kernel "keeps low frequencies and rejects high
frequencies better than any (achievable) filter we've seen
so far."

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
following sample positions are used. The final -0.5 term in
the x equation causes the s2() samples to be shifted
slightly. For example, when downsampling by a factor of two
(e.g. n1 = 10 and n2 = 5), the first sample position
x[0] = 0.5. However, this sample corresponds to the input
range [0,2) so one might expect that the first sample
position should be x[0] = 1.0. I was unable to find a
definitive explaination, however, it seems likely that this
approach is useful in reducing aliasing and/or ringing
artifacts. Also, if the pixel shift had not been included
then downsampling would simply match nearest neighbor
interpolation since L(0.0) = 1.0 and is zero for all other
integer inputs of L(x).

	step = n1/n2
	j    = [0..n2)
	x    = (j + 0.5)*step - 0.5

When downsampling, the support size (a) is typically scaled
by the filter scale (fs = n1/n2). I was unable to find a
definitive explaination, however, it seems likely that this
approach is necessary to filter high frequency components
from the input signal. When upscaling, the support size (a)
does not need to be scaled as high frequency components are
not being filtered.

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
* [Lanczos interpolation and resampling](https://www.youtube.com/watch?v=ijmd6XyG2HA)
* [Interpolation Algorithms](https://pixinsight.com/doc/docs/InterpolationAlgorithms/InterpolationAlgorithms.html)
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
