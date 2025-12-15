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

#ifndef lanczos_resample_H
#define lanczos_resample_H

#include <stdint.h>

// Edge Handling
// default: CLAMPING
#define LANCZOS_FLAG_EDGE_ZERO_PADDING 0x0001
#define LANCZOS_FLAG_EDGE_CLAMPING     0x0002

// Multidimensional Interpolation
// default: 2D_SEPARABLE
#define LANCZOS_FLAG_MULTIDIM_2D_SEPARABLE 0x0010
#define LANCZOS_FLAG_MULTIDIM_2D_ISOTROPIC 0x0020

// Irregular Data Holes
// default: WEIGHTED
#define LANCZOS_FLAG_NODATA_ZERO     0x0100
#define LANCZOS_FLAG_NODATA_NEAREST  0x0200
#define LANCZOS_FLAG_NODATA_WEIGHTED 0x0400

typedef struct
{
	uint32_t flags;
	uint32_t a;
	uint32_t channels;
	uint32_t src_w;
	uint32_t dst_w;
	float* src;
	float* dst;
} lanczos_paramRegular1D_t;

typedef struct
{
	uint32_t flags;
	uint32_t a;
	uint32_t channels;
	uint32_t src_w;
	uint32_t src_h;
	uint32_t dst_w;
	uint32_t dst_h;
	float* src;
	float* dst;
} lanczos_paramRegular2D_t;

typedef struct
{
	uint32_t flags;
	uint32_t a;
	uint32_t channels;
	uint32_t src_count;
	float    src_x0;
	float    src_x1;
	uint32_t dst_w;
	float*   src;
	float*   dst;
} lanczos_paramIrregular1D_t;

typedef struct
{
	uint32_t flags;
	uint32_t a;
	uint32_t channels;
	uint32_t src_count;
	float    src_x0;
	float    src_y0;
	float    src_x1;
	float    src_y1;
	uint32_t dst_w;
	uint32_t dst_h;
	float*   src;
	float*   dst;
} lanczos_paramIrregular2D_t;

int lanczos_resample_regular1D(lanczos_paramRegular1D_t* param);
int lanczos_resample_regular2D(lanczos_paramRegular2D_t* param);
int lanczos_resample_irregular1D(lanczos_paramIrregular1D_t* param);
int lanczos_resample_irregular2D(lanczos_paramIrregular2D_t* param);

#endif
