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

#ifndef lanczos_resampler_H
#define lanczos_resampler_H

#include "lanczos_param.h"
#include "lanczos_signal.h"

typedef struct
{
	float*           lcoef;
	float*           w;
	lanczos_signal_t sx;
	lanczos_signal_t sy;
} lanczos_resampler_t;

lanczos_resampler_t* lanczos_resampler_new(void);
void                 lanczos_resampler_delete(lanczos_resampler_t** _self);
void                 lanczos_resampler_discard(lanczos_resampler_t* self);
int                  lanczos_resampler_apply(lanczos_resampler_t* self,
                                             lanczos_param_t* param,
                                             lanczos_signal_t* s1,
                                             lanczos_signal_t* s2);

#endif
