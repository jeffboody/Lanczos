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

#ifndef lanczos_signal_H
#define lanczos_signal_H

#include <stdint.h>

typedef struct
{
	uint32_t channels;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	float*   data;
} lanczos_signal_t;

lanczos_signal_t* lanczos_signal_new(uint32_t channels,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth);
void              lanczos_signal_delete(lanczos_signal_t** _self);
void              lanczos_signal_discard(lanczos_signal_t* self);
int               lanczos_signal_copy(lanczos_signal_t* s1,
                                      lanczos_signal_t* s2);
size_t            lanczos_signal_sizeBytes(lanczos_signal_t* self);
int               lanczos_signal_resize(lanczos_signal_t* self);

/*
 * signal access functions
 */

static inline float
lanczos_signal_get(lanczos_signal_t* self,
                   uint32_t ch, uint32_t i,
                   uint32_t j, uint32_t k)
{
	ASSERT(self);

	uint32_t si  = self->channels;
	uint32_t sj  = si*self->width;
	uint32_t sk  = sj*self->height;
	uint32_t idx = sk*k + sj*j + si*i + ch;

	return self->data[idx];
}

static inline void
lanczos_signal_set(lanczos_signal_t* self,
                   uint32_t ch, uint32_t i,
                   uint32_t j, uint32_t k,
                   float val)
{
	ASSERT(self);

	uint32_t si  = self->channels;
	uint32_t sj  = si*self->width;
	uint32_t sk  = sj*self->height;
	uint32_t idx = sk*k + sj*j + si*i + ch;

	self->data[idx] = val;
}

#endif
