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

#include <stdlib.h>
#include <string.h>

#define LOG_TAG "lanczos"
#include "../../libcc/cc_log.h"
#include "../../libcc/cc_memory.h"
#include "lanczos_signal.h"

/*
 * public
 */

lanczos_signal_t*
lanczos_signal_new(uint32_t channels, uint32_t width,
                   uint32_t height, uint32_t depth)
{
	uint32_t count = channels*width*height*depth;
	if(count == 0)
	{
		LOGE("invalid count=%u", count);
		return NULL;
	}

	lanczos_signal_t* self;
	self = (lanczos_signal_t*)
	       CALLOC(1, sizeof(lanczos_signal_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->channels = channels;
	self->width    = width;
	self->height   = height;
	self->depth    = depth;

	self->data = (float*) CALLOC(count, sizeof(float));
	if(self->data == NULL)
	{
		LOGE("CALLOC failed");
		goto failure;
	}

	// success
	return self;

	// failure
	failure:
		lanczos_signal_delete(&self);
	return NULL;
}

void lanczos_signal_delete(lanczos_signal_t** _self)
{
	ASSERT(_self);

	lanczos_signal_t* self = *_self;
	if(self)
	{
		lanczos_signal_discard(self);
		FREE(self);
		*_self = NULL;
	}
}

void lanczos_signal_discard(lanczos_signal_t* self)
{
	ASSERT(self);

	FREE(self->data);
	self->data = NULL;
}

int lanczos_signal_copy(lanczos_signal_t* s1,
                        lanczos_signal_t* s2)
{
	ASSERT(s1);
	ASSERT(s2);

	// validate input
	if((s1->channels != s2->channels) ||
	   (s1->width    != s2->width)    ||
	   (s1->height   != s2->height)   ||
	   (s1->depth    != s2->depth))
	{
		LOGE("invalid channels=%u:%u, width=%u:%u, height=%u:%u, depth=%u:%u",
		     s1->channels, s2->channels,
		     s1->width, s2->width,
		     s1->height, s2->height,
		     s1->depth, s2->depth);
		return 0;
	}

	// validate s1 data
	size_t copy_size = lanczos_signal_sizeBytes(s1);
	size_t data_size = MEMSIZEPTR(s1->data);
	if(data_size < copy_size)
	{
		LOGE("invalid copy_size=%u, data_size=%u",
		     (uint32_t) copy_size, (uint32_t) data_size);
		return 0;
	}

	// validate s2 data
	if(lanczos_signal_resize(s2) == 0)
	{
		return 0;
	}

	// copy data
	memcpy(s2->data, s1->data, copy_size);
	return 1;
}

size_t lanczos_signal_sizeBytes(lanczos_signal_t* self)
{
	ASSERT(self);

	return self->channels*self->width*self->height*
	       self->depth*sizeof(float);
}

int lanczos_signal_resize(lanczos_signal_t* self)
{
	ASSERT(self);

	size_t signal_size = lanczos_signal_sizeBytes(self);
	size_t data_size = MEMSIZEPTR(self->data);
	if(data_size < signal_size)
	{
		float* data = REALLOC(self->data, signal_size);
		if(data == NULL)
		{
			LOGE("REALLOC failed");
			return 0;
		}
		self->data = data;
	}

	return 1;
}
