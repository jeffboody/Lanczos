Lanczos Resampling
==================

Introduction
------------

Lanczos resampling is a sophisticated technique for
interpolating digital signals, offering superior image
quality compared to simpler methods like nearest neighbor
and bilinear interpolation. By effectively preserving detail
and minimizing aliasing artifacts, Lanczos resampling is
widely used in image and signal processing applications.

While Lanczos excels in image quality, it comes at the cost
of increased computational complexity. Additionally, the
potential for "ringing" artifacts, particularly around sharp
edges, can be a drawback. Despite these challenges, Lanczos
remains a preferred choice for tasks such as image scaling
and rotation due to its overall quality benefits.

This document provides a comprehensive overview of Lanczos
resampling, including its theoretical underpinnings,
implementation details, and practical applications. It
serves as a valuable resource for developers and researchers
seeking to understand and utilize this powerful technique.
The subsequent sections delve into crucial aspects of
Lanczos resampling, such as the Lanczos kernel,
interpolation and resampling processes, flux preservation,
upsampling, downsampling, sample positioning, output range,
multidimensional interpolation, separability, and precomputed
kernel optimization. A practical example with accompanying
source code is included.

Lanczos Kernel
--------------

The
[Lanczos kernel](https://en.wikipedia.org/wiki/Lanczos_resampling),
defined for a given support size, is a function used in
Lanczos resampling. The kernel is defined as:

	L(x) = sinc(x)*sinc(x/a) : -a < x < a
	     = 0.0               : otherwise

Where:

* x is an independent variable evaluated over the Lanczos
  window
* a is the support size of the kernel, controlling the width

The normalized sinc function is defined as:

	sinc(x) = 1.0              : x = 0.0
	        = sin(PI*x)/(PI*x) : otherwise

![Lanczos3 Kernel](lanczos3.jpg?raw=true "Lanczos3 Kernel")

The graph illustrates the shape of the Lanczos kernel for a
support size of a = 3. This means the kernel includes three
lobes of the sinc function. While increasing the support
size (a) generally provides more flexibility for shaping the
frequency response, it also increases computational cost. A
balance must be struck between image quality and
computational efficiency when choosing the support size. Jim
Blinn's finding that the Lanczos kernel with a = 3 offers an
excellent balance of low-frequency preservation and
high-frequency rejection supports this notion.

Note: The terms "support size," "kernel width," and
"filter size" are often used interchangeably to describe the
parameter a. However, in the context of Lanczos resampling,
"support size" most accurately reflects the concept.

Lanczos Interpolation and Resampling
------------------------------------

Lanczos interpolation is a technique used to resample a
discrete signal to a new sampling rate. It achieves this by
convolving the original signal with a Lanczos kernel.

The interpolated signal s2 is calculated as a weighted sum
of the original samples:

	s2[j] = (1/wj)*
	        SUM(i = i0, i = i1, s1(floor(xj) + i)*
	            L((i - xj + floor(xj))/fs))

Preserving Flux:

The normalization factor wj is crucial for preserving
signal energy (mass). It ensures that the sum of the weights
equals 1, preventing the interpolated signal from becoming
globally brighter or darker.

	wj = SUM(i = i0, i = i1, L((i - xj + floor(xj))/fs))

Upsampling:

When increasing the sampling rate, the filter scale is equal
to one since the kernel radius is equal to the support
radius (e.g. no stretching).

	fs = 1

The summation bounds cover a fixed window of 2\*a samples.

	i0 = -a + 1
	i1 = a

Downsampling:

To avoid aliasing artifacts when decreasing the sampling
rate, the filter scale must be adjusted to match the new
sampling rate.

	fs = n1/n2

When the support radius (fs\*a) is an integer value, the
summation bounds covers a fixed window of 2\*fs\*a samples.

	i0 = -fs*a + 1
	i1 = fs*a

However, when the support radius (fs\*a) is not an integer
value then the summation bounds requires a dynamic window.
The dynamic window is required because as xj moves across
the input signal, the number of integer points covered by
the stretched kernel may vary depending on the position of
xj.

	i0 = floor(-fs*a + 1 + (xj - floor(xj)))
	i1 = floor(fs*a + (xj - floor(xj)))

Sample Positions:

When resampling a signal from n1 samples to n2 samples the
following sample positions are used. The index j is used to
represent the samples from the sampled signal s2. The term
(j + 0.5) is the center of a sample in s2. The step scales a
point in s2 to a point in s1. The final -0.5 term in xj is a
phase shift that causes the Lanczos coefficient samples to
be offset.

	step = n1/n2
	j    = [0..n2)
	xj   = (j + 0.5)*step - 0.5

Edge Handling
-------------

When performing Lanczos interpolation, special care must be
taken at the signal boundaries. Common edge handling
techniques include:

* Zero padding: Extending the signal with zeros outside its
  original range.
* Clamping: Assigning the edge values to samples outside the
  signal boundaries.
* Mirroring: Reflecting the signal at the boundaries.
* Repeating: Repeating the edge values multiple times.

Zero Padding:

	s1(x) = s1[floor(x)] : x = [0, n1)
	      = 0.0          : otherwise

Clamping:

	s1(x) = s1[clamp(floor(x), 0, n1 - 1)]

Clamping is often preferred as it reduces edge artifacts
caused by sharp discontinuities near the edges. However, the
choice of edge handling method depends on the specific
application and desired output.

Output Range
------------

The range of s2 may be greater than that of s1 due to the
lobes of L(x). For example, an input signal s1 corresponding
to pixel colors in the range of [0.0,1.0] need to be clamped
to the same range to ensure that the output values do not
overflow when converted back to unsigned bytes of [0,255].

Multichannel Data
-----------------

When a dataset, such as a color image, consists of
multichannel data, the standard procedure is to treat each
channel independently.

Multidimensional Interpolation
------------------------------

The Lanczos kernel can be extended to multiple dimensions to
perform interpolation on images, volumetric data, or
higher-dimensional signals. While full multidimensional
Lanczos equations exist that operate simultaneously across
all dimensions using radial distance, they carry significant
computational overhead and are rarely used in practical
implementations. Instead, multidimensional resampling is
commonly performed using separable approximations.

Separability
------------

To improve performance, practical implementations
approximate multidimensional Lanczos filtering by applying
the one-dimensional kernel sequentially along each axis
(e.g., horizontal, vertical, and depth passes).

Horizontal Interpolation (X-axis):

Apply the 1D Lanczos kernel along each row of the input
signal s1 to produce intermediate result s2.

	s2(x, y, z) = (1/w(x))*
	              SUM(i = i0, i = i1,
	                  s1(floor(x) + i, y, z)*
	                  L((i - x + floor(x))/fs))

Vertical Interpolation (Y-axis):

Apply the 1D Lanczos kernel along each column of the input
signal s2 to produce intermediate result s3.

	s3(x, y, z) = (1/w(y))*
	              SUM(j = j0, j = j1,
	                  s2(x, floor(y) + j, z)*
	                  L((j - y + floor(y))/fs))

Depth Interpolation (Z-axis):

Apply the 1D Lanczos kernel along the depth axis of s3 to
produce the final interpolated signal s4.

	s4(x, y, z) = (1/w(z))*
	              SUM(k = k0, k = k1,
	                  s3(x, y, floor(z) + k)*
	                  L((k - z + floor(z))/fs))

Note that the normalization factors w(x), w(y), and w(z) are
calculated independently using the one-dimensional Lanczos
kernel for each respective axis.

Key Points:

* Efficiency: The separable approximation reduces the time
  complexity for an N-dimensional kernel of support size 'a'
  from O((2*a)^N) per output sample down to O(N*2*a).
* Trade-offs: Separable filtering introduces minor
  rotational anisotropic artifacts compared to the full
  multidimensional Lanczos interpolation, but the massive
  gain in performance makes it the standard approach in
  practice.
* Cumulative Errors: Recursively resampling a signal
  multiple times (such as generating texture mipmap chains)
  degrades signal quality due to error accumulation at each
  sequential pass.

Precomputed Kernel Optimization
-------------------------------

The Lanczos kernel is computationally expensive because the
sinc function requires trigonometric sine calls. This cost
can be eliminated through precomputation when using specific
resampling factors.

Upsampling (Fast Path):

When upsampling by an integer factor S, the Lanczos
coefficients repeat in S phases where each phase includes
2\*a coefficients.

* Resampling Factor: S = n2/n1
* Total Coefficients: N = S\*2\*a
* Runtime Logic: The algorithm selects the correct
  precomputed phase using j % S.

Downsampling (Fast Path):

When downsampling by a factor S that is the reciprocal of an
integer D, the Lanczos coefficients repeat for every output
sample resulting in a single phase with D\*2\*a
coefficients.

* Resampling Factor: S = 1/D = n2/n1
* Total Coefficients: N = D\*2\*a
* Runtime Logic: The same single set of Lanczos coefficients
  is applied to every output sample.

Arbitrary Resampling (Slow Path):

When the resampling factor does not adhere to these specific
integer or reciprocal conditions, the algorithm must fall
back to a "slow path." In this mode, the Lanczos
coefficients are computed at runtime for every individual
sample to accommodate the non-repeating values.

Example: 1D Sine Test
---------------------

Run the sine-test example and use gnuplot to see how
Lanczos resampling affects a simple one-dimensional signal
when upsampled and downsampled by various factors.

	cd sine-test
	./setup.sh
	make -j4
	./run.sh
	gnuplot
	> load "sine-test.plot"

![1D Sine Test](sine-test/sine-test.jpg?raw=true "1D Sine Test")

References
----------

This README was created with the assistance of
[Google Gemini](https://gemini.google.com/).

Additional references include:

* [Lanczos Resampling](https://en.wikipedia.org/wiki/Lanczos_resampling)
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
