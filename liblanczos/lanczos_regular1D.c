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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "lanczos"
#include "../../libcc/cc_list.h"
#include "../../libcc/cc_log.h"
#include "../../libcc/cc_memory.h"
#include "lanczos_flags.h"
#include "lanczos_regular1D.h"
#include "lanczos_kernel.h"

/*
 * private
 */

static int
lanczos_regular1D_resampleFast(lanczos_regular1D_t* self,
                               int32_t phases, int32_t N,
                               int32_t fs)
{
	ASSERT(self);

	float* lcoef = (float*) CALLOC(N, sizeof(float));
	if(lcoef == NULL)
	{
		LOGE("CALLOC failed");
		return 0;
	}

	float* wj = (float*) CALLOC(phases, sizeof(float));
	if(wj == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_wj;
	}

	float xj;
	float x;
	float step = ((float) self->src_w)/
	             ((float) self->dst_w);

	// precompute Lanczos kernel coefficients and
	// normalizing weights
	int32_t i;
	int32_t j;
	int32_t idx = 0;
	int32_t i0  = -((int32_t) fs*self->a) + 1;
	int32_t i1  = (int32_t) fs*self->a;
	for(j = 0; j < phases; ++j)
	{
		xj    = (j + 0.5f)*step - 0.5f;
		wj[j] = 0.0f;
		for(i = i0; i <= i1; ++i)
		{
			if(idx >= N)
			{
				LOGE("invalid idx=%i, N=%u", idx, N);
				goto fail_precompute;
			}

			x          = (i - xj + floorf(xj))/fs;
			lcoef[idx] = lanczos_kernel(x, self->a);
			wj[j]     += lcoef[idx];
			++idx;
		}
	}

	if(idx != N)
	{
		LOGE("invalid idx=%i, N=%u", idx, N);
		goto fail_precompute;
	}

	// commpute s2[j]
	int32_t ch;
	int32_t nch = self->channels;
	int32_t ii;
	int32_t jj;
	int32_t s1x;
	float   sum;
	float*  s1 = self->src;
	float*  s2 = self->dst;
	for(ch = 0; ch < nch; ++ch)
	{
		// reset idx
		idx = 0;

		for(j = 0; j < self->dst_w; ++j)
		{
			sum = 0.0f;
			jj  = j%phases;
			xj  = (j + 0.5f)*step - 0.5f;
			for(i = i0; i <= i1; ++i)
			{
				// Edge Handling
				s1x = ((int32_t) floorf(xj)) + i;
				if(self->flags & LANCZOS_FLAG_EDGE_ZERO_PADDING)
				{
					// Zero Padding
					if((s1x < 0) || (s1x >= (self->src_w - 1)))
					{
						++idx;
						continue;
					}
				}
				else
				{
					// Clamping
					if(s1x < 0)
					{
						s1x = 0;
					}
					else if(s1x >= self->src_w)
					{
						s1x = self->src_w - 1;
					}
				}

				ii   = idx%N;
				sum += s1[nch*s1x + ch]*lcoef[ii];
				++idx;
			}

			// Preserving Flux Normalization
			s2[nch*j + ch] = sum/wj[jj];
		}
	}

	FREE(wj);
	FREE(lcoef);

	// success
	return 1;

	// failure
	fail_precompute:
		FREE(wj);
	fail_wj:
		FREE(lcoef);
	return 0;
}

static int
lanczos_regular1D_resampleSlow(lanczos_regular1D_t* self)
{
	ASSERT(self);

	float fs = 1.0f;
	if(self->dst_w < self->src_w)
	{
		fs = ((float) self->src_w)/((float) self->dst_w);
	}

	float step = ((float) self->src_w)/
	             ((float) self->dst_w);

	// commpute s2[j]
	int32_t ch;
	int32_t nch = self->channels;
	int32_t j;
	int32_t i;
	int32_t i0;
	int32_t i1;
	int32_t s1x;
	float   sum;
	float   xj;
	float   wj;
	float   lcoef;
	float*  s1 = self->src;
	float*  s2 = self->dst;
	for(ch = 0; ch < nch; ++ch)
	{
		for(j = 0; j < self->dst_w; ++j)
		{
			sum = 0.0f;
			wj  = 0.0f;
			xj  = (j + 0.5f)*step - 0.5f;
			i0 = (int32_t) floorf(-fs*self->a + 1 + (xj - floorf(xj)));
			i1 = (int32_t) floorf(fs*self->a + (xj - floorf(xj)));
			for(i = i0; i <= i1; ++i)
			{
				// Edge Handling
				s1x = ((int32_t) floorf(xj)) + i;
				if(self->flags & LANCZOS_FLAG_EDGE_ZERO_PADDING)
				{
					// Zero Padding
					if((s1x < 0) || (s1x >= (self->src_w - 1)))
					{
						continue;
					}
				}
				else
				{
					// Clamping
					if(s1x < 0)
					{
						s1x = 0;
					}
					else if(s1x >= self->src_w)
					{
						s1x = self->src_w - 1;
					}
				}

				lcoef = lanczos_kernel((i - xj + floor(xj))/fs,
				                       self->a);
				sum  += s1[nch*s1x + ch]*lcoef;
				wj   += lcoef;
			}

			// Preserving Flux Normalization
			s2[nch*j + ch] = sum/wj;
		}
	}

	return 1;
}

/*
 * public
 */

int lanczos_regular1D_resample(lanczos_regular1D_t* self)
{
	ASSERT(self);
	ASSERT(self->src);
	ASSERT(self->dst);

	// detect resampling fast paths
	if(self->dst_w >= self->src_w)
	{
		if((self->dst_w % self->src_w) == 0)
		{
			// Upsampling (Fast Path)
			// Resampling Factor: S = n2/n1
			// Total Coefficients: N = S*2*a
			// Filter Scale: 1
			int32_t S  = self->dst_w/self->src_w;
			int32_t N  = S*2*self->a;
			int32_t fs = 1;
			return lanczos_regular1D_resampleFast(self, S, N, fs);
		}
	}
	else if((self->src_w % self->dst_w) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		int32_t D  = self->src_w/self->dst_w;
		int32_t N  = D*2*self->a;
		int32_t fs = D;
		return lanczos_regular1D_resampleFast(self, 1, N, fs);
	}

	// Arbitrary Resampling (Slow Path)
	return lanczos_regular1D_resampleSlow(self);
}
