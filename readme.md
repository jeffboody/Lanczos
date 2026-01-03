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
multidimensional interpolation, separability, precomputed
kernel optimization, and irregular data. A practical example
with accompanying source code is included.

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

Note: The terms "kernel width," "support size," and
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

Sample Positions
----------------

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
perform interpolation on images or higher-dimensional data.

The two-dimensional Lanczos kernel is defined as:

	L(x, y) = sinc(sqrt(x^2 + y^2))*sinc(sqrt(x^2 + y^2)/a)

The interpolated signal s2 can be calculated using the
following formula:

	s2(x, y) = (1/w(x, y))*
	           SUM(i = i0, i = i1,
	               SUM(j = j0, j = j1,
	                   s1(floor(x) + j, floor(y) + i)*
	                   L((j - x + floor(x))/fs,
	                     (i - y + floor(y))/fs)))

Where w(x, y) is the normalization factor calculated using
the two-dimensional Lanczos kernel.

Separability
------------

Unlike some interpolation methods, the Lanczos kernel is
non-separable, meaning it cannot be factored into the
product of one-dimensional kernels. This property generally
leads to higher computational costs compared to separable
kernels. To improve performance, some implementations
approximate the Lanczos kernel by performing separate passes
for the horizontal and vertical dimensions.

Horizontal Interpolation:

* Apply the one-dimensional Lanczos kernel to each row of
  the input signal s1.
* This produces an intermediate result, s2.

Vertical Interpolation:

* Apply the one-dimensional Lanczos kernel to each column of
  the intermediate result s2.
* This produces the final interpolated signal, s3.

Mathematical Representation:

	s2(x, y) = (1/w(x))*
	           SUM(j = j0, j = j1,
	               s1(floor(x) + j, y)*
	               L((j - x + floor(x))/fs))

	s3(x, y) = (1/w(y))*
	           SUM(i = i0, i = i1,
	               s2(x, floor(y) + i)*
	               L((i - y + floor(y))/fs))

Note that the normalization factors w(x) and w(y) are
calculated using the one-dimensional Lanczos kernel.

Key Points:

* The separable approximation significantly reduces
  computational cost compared to the full two-dimensional
  Lanczos interpolation.
* However, it introduces artifacts due to the loss of
  information from the non-separable components of the
  Lanczos kernel.
* The choice between the full Lanczos interpolation and the
  separable approximation depends on the specific
  application's requirements for quality and computational
  efficiency.

Similarly, recursively resampling a signal multiple times,
as commonly done for generating texture mipmaps, can also
degrade image quality due to the accumulation of errors
from each resampling step.

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

Irregular Data
--------------

Lanczos irregular interpolation is a technique used to
resample irregularly spaced samples into a regular grid. It
achieves this by assigning the irregularly spaced samples to
a regular grid, a procedure often called binning, followed
by convolution with a Lanczos kernel.

Irregular Interpolation and Normalization:

Once again, the interpolated signal s2 is calculated as a
weighted sum of the original samples. However, an additional
density compensation weight vj is required to account for
the varying sample density of irregularly spaced samples.
The irregular sample positions must also be transformed to
a regular grid space via a mapping function.

	s2[j] = (1/wj)*
	        SUM(i = 0, i = N - 1, vj*s1[i]*L(xi2jf(xi) - j))

	wj = SUM(i = 0, i = N - 1, vj*L(xi2jf(xi) - j))

In practice, only samples which were assigned to grid cells
within the support radius are considered.

The Mapping Function:

The functions xi2ji(xi) and xi2jf(xi) map a sample position
from the irregular coordinate space to an index (integer or
floating point) in the regular grid space. Given the bounds
of the irregular space [x0,x1], the mapping is defined as:

	xi2jf(xi) => jf = n2*(xi - x0)/(x1 - x0)
	xi2ji(xi) => ji = (int) floor(xi2jf(xi))

When xi=x1 then j=n2 which is out-of-bounds for s2[j].

The Inverse Mapping Function:

The functions ji2xi(ji) and jf2xi(jf) map an index (integer
or floating point) in the regular grid space back to a
sample position in the irregular coordinate space.

	jf2xi(jf) => xi = x0 + (x1 - x0)*jf/n2
	ji2xi(ji) => xi = jf2xi(ji + 0.5f)

For example, given x0=0, x1=1 and n2=4.

	       x0 <-------- xi ------> x1
	        0    1/4   1/2   3/4    1
	+-------+-----+-----+-----+-----+
	| ji    |  0  |  1  |  2  |  3  |
	| ji2xi | 1/8 | 3/8 | 5/8 | 7/8 |
	+-------+-----+-----+-----+-----+

Density Compensation:

In many irregular datasets, samples are often clustered in
specific regions. Without correction, these high-density
clusters will disproportionately influence the interpolated
result. To correct this, a density compensation weight vj
is assigned to each regular grid cell. Samples contributing
to dense cells receive a low weight, while those in isolated
areas receive a high weight.

The density weight for a regular grid cell j is calculated
by summing the Lanczos kernel values for all irregular
samples xi that fall within that cell's support radius.

	vj = 1/(SUM(i = 0, i = N - 1, L(xi2jf(xi) - j)))

Special Case Handling:

* Zero-Weight Cancellations: Because the Lanczos kernel
  contains negative lobes, it is mathematically possible for
  a set of samples to yield a total sum of zero. To prevent
  division by zero in this case, the implementation should
  check if ∣SUM∣ < epsilon before calculating the
  reciprocal.
* Zero-Density Cells: When no samples exist within the
  support radius, the summation is zero. In this case, vj is
  typically set to 0 (zero-fill), resulting in an empty or
  "null" region in the output grid.
* Continuous Fallback: For applications requiring a
  continuous signal, if the support window is empty, the
  implementation may fallback to a nearest-neighbor search
  or linear interpolation to fill the gap.

Aliasing and Bandwidth:

The Lanczos kernel assumes the input is a band-limited
signal sampled at or above the Nyquist rate. With irregular
spacing, this assumption is not guaranteed. The quality of
the result depends heavily on the local sampling density. In
areas where samples are sparse, the kernel may fail to
capture high-frequency features, leading to artifacts.

Computational Cost:

Processing irregular data is significantly more expensive
than regular data because:

* Search Overhead: The algorithm must locate which irregular
  samples xi fall within the support window of the output
  point j. This is typically handled by binning samples
  into a spatial data structure.
* No Precomputation: Since the distances between samples are
  arbitrary, the Lanczos coefficients cannot be precomputed
  into a repeating lookup table.
* Density Compensation: Extra processing is required to
  normalize density for irregularly spaced samples.

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
