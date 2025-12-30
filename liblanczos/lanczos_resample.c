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

#define LOG_TAG "lanczos"
#include "../../libcc/cc_log.h"
#include "../../libcc/cc_memory.h"
#include "lanczos_resample.h"

/*
 * private
 */

static float sinc(float x)
{
	if(x == 0.0f)
	{
		return 1.0f;
	}

	return sinf(M_PI*x)/(M_PI*x);
}

static float L(float x, float a)
{
	if((-a < x) && (x < a))
	{
		return sinc(x)*sinc(x/a);
	}

	return 0.0f;
}

static int
lanczos_resample_regular1DFast(lanczos_paramRegular1D_t* param,
                               uint32_t phases, uint32_t N,
                               uint32_t fs)
{
	ASSERT(param);

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
	float step = ((float) param->src_w)/
	             ((float) param->dst_w);

	// precompute Lanczos kernel coefficients and
	// normalizing weights
	int32_t i;
	int32_t j;
	int32_t idx = 0;
	int32_t i0  = -((int32_t) fs*param->a) + 1;
	int32_t i1  = (int32_t) fs*param->a;
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
			lcoef[idx] = L(x, param->a);
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
	uint32_t ch;
	uint32_t nch = param->channels;
	int32_t  ii;
	int32_t  jj;
	int32_t  s1x;
	float    sum;
	float*   s1 = param->src;
	float*   s2 = param->dst;
	for(ch = 0; ch < nch; ++ch)
	{
		// reset idx
		idx = 0;

		for(j = 0; j < param->dst_w; ++j)
		{
			sum = 0.0f;
			jj  = j%phases;
			xj  = (j + 0.5f)*step - 0.5f;
			for(i = i0; i <= i1; ++i)
			{
				// Edge Handling
				s1x = ((int32_t) floorf(xj)) + i;
				if(param->flags & LANCZOS_FLAG_EDGE_ZERO_PADDING)
				{
					// Zero Padding
					if((s1x < 0) || (s1x >= (param->src_w - 1)))
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
					else if(s1x >= param->src_w)
					{
						s1x = param->src_w - 1;
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

/*
 * public
 */

int lanczos_resample_regular1D(lanczos_paramRegular1D_t* param)
{
	ASSERT(param);
	ASSERT(param->src);
	ASSERT(param->dst);

	// detect resampling fast paths
	if(param->dst_w >= param->src_w)
	{
		if((param->dst_w % param->src_w) == 0)
		{
			// Upsampling (Fast Path)
			// Resampling Factor: S = n2/n1
			// Total Coefficients: N = S*2*a
			// Filter Scale: 1
			uint32_t S  = param->dst_w/param->src_w;
			uint32_t N  = S*2*param->a;
			uint32_t fs = 1;
			return lanczos_resample_regular1DFast(param, S, N, fs);
		}
	}
	else if((param->src_w % param->dst_w) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		uint32_t D  = param->src_w/param->dst_w;
		uint32_t N  = D*2*param->a;
		uint32_t fs = D;
		return lanczos_resample_regular1DFast(param, 1, N, fs);
	}

	// TODO - Arbitrary Resampling (Slow Path)

	return 0;
}

int lanczos_resample_regular2D(lanczos_paramRegular2D_t* param)
{
	ASSERT(param);
	ASSERT(param->src);
	ASSERT(param->dst);

	// TODO - lanczos_resample_regular2D
	return 0;
}

int lanczos_resample_irregular1D(lanczos_paramIrregular1D_t* param)
{
	ASSERT(param);
	ASSERT(param->src);
	ASSERT(param->dst);

	// TODO - lanczos_resample_irregular1D
	return 0;
}

int lanczos_resample_irregular2D(lanczos_paramIrregular2D_t* param)
{
	ASSERT(param);
	ASSERT(param->src);
	ASSERT(param->dst);

	// TODO - lanczos_resample_irregular2D
	return 0;
}
