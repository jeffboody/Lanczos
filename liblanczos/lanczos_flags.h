/*
 * Copyright (c) 2025 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef lanczos_flags_H
#define lanczos_flags_H

// Edge Handling
// default: CLAMPING
#define LANCZOS_FLAG_EDGE_ZERO_PADDING 0x0001
#define LANCZOS_FLAG_EDGE_CLAMPING     0x0002

// Multidimensional Interpolation
// default: 2D_SEPARABLE
#define LANCZOS_FLAG_MULTIDIM_2D_SEPARABLE 0x0010
#define LANCZOS_FLAG_MULTIDIM_2D_ISOTROPIC 0x0020

// Irregular Data Holes
// default: LINEAR
#define LANCZOS_FLAG_NODATA_ZERO     0x0100
#define LANCZOS_FLAG_NODATA_NEAREST  0x0200
#define LANCZOS_FLAG_NODATA_LINEAR   0x0400
#define LANCZOS_FLAG_NODATA_MASK     0x0700

#endif
