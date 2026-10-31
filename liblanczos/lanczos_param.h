/*
 * Copyright (c) 2026 Jeff Boody
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

#ifndef lanczos_param_H
#define lanczos_param_H

#include <stdint.h>

typedef enum
{
	LANCZOS_EDGE_MODE_ZERO  = 0,
	LANCZOS_EDGE_MODE_CLAMP = 1,
} lanczos_edgeMode_e;

typedef enum
{
	LANCZOS_CLAMP_MODE_DISABLE = 0,
	LANCZOS_CLAMP_MODE_ENABLE  = 1,
} lanczos_clampMode_e;

typedef struct
{
	uint32_t            a;
	lanczos_edgeMode_e  edge_mode;
	lanczos_clampMode_e clamp_mode;
	float               clamp_min;
	float               clamp_max;
} lanczos_param_t;

#endif
