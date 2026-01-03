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
#include "lanczos_resample.h"

typedef struct
{
	int32_t     bin_count;
	cc_list_t** bins;
	cc_list_t*  holes;
} lanczos_irregularState_t;

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

static void
lanczos_irregularState_discard(lanczos_irregularState_t* state)
{
	ASSERT(state);

	cc_listIter_t* iter;
	if(state->holes)
	{
		iter = cc_list_head(state->holes);
		while(iter)
		{
			FREE((float*) cc_list_remove(state->holes, &iter));
		}
		cc_list_delete(&state->holes);
	}

	int32_t i;
	if(state->bins)
	{
		for(i = 0; i < state->bin_count; ++i)
		{
			cc_list_delete(&state->bins[i]);
		}
		FREE(state->bins);
		state->bins = NULL;
		state->bin_count = 0;
	}
}

static int
lanczos_irregularState_init(lanczos_irregularState_t* state,
                            int32_t bin_count)
{
	ASSERT(state);

	// create holes
	state->holes = cc_list_new();
	if(state->holes == NULL)
	{
		return 0;
	}

	// create bins
	state->bins = (cc_list_t**)
	              CALLOC(bin_count, sizeof(cc_list_t*));
	if(state->bins == NULL)
	{
		LOGE("CALLOC failed");
		goto failure;
	}

	// create bin lists
	int32_t i;
	for(i = 0; i < bin_count; ++i)
	{
		state->bins[i] = cc_list_new();
		if(state->bins[i] == NULL)
		{
			goto failure;
		}
	}

	state->bin_count = bin_count;

	// success
	return 1;

	// failure
	failure:
		lanczos_irregularState_discard(state);
	return 0;
}

static int
lanczos_resample_regular1DFast(lanczos_paramRegular1D_t* param,
                               int32_t phases, int32_t N,
                               int32_t fs)
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
	int32_t ch;
	int32_t nch = param->channels;
	int32_t ii;
	int32_t jj;
	int32_t s1x;
	float   sum;
	float*  s1 = param->src;
	float*  s2 = param->dst;
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

static int
lanczos_resample_regular1DSlow(lanczos_paramRegular1D_t* param)
{
	ASSERT(param);

	float fs = 1.0f;
	if(param->dst_w < param->src_w)
	{
		fs = ((float) param->src_w)/((float) param->dst_w);
	}

	float step = ((float) param->src_w)/
	             ((float) param->dst_w);

	// commpute s2[j]
	int32_t ch;
	int32_t nch = param->channels;
	int32_t j;
	int32_t i;
	int32_t i0;
	int32_t i1;
	int32_t s1x;
	float   sum;
	float   xj;
	float   wj;
	float   lcoef;
	float*  s1 = param->src;
	float*  s2 = param->dst;
	for(ch = 0; ch < nch; ++ch)
	{
		for(j = 0; j < param->dst_w; ++j)
		{
			sum = 0.0f;
			wj  = 0.0f;
			xj  = (j + 0.5f)*step - 0.5f;
			i0 = (int32_t) floorf(-fs*param->a + 1 + (xj - floorf(xj)));
			i1 = (int32_t) floorf(fs*param->a + (xj - floorf(xj)));
			for(i = i0; i <= i1; ++i)
			{
				// Edge Handling
				s1x = ((int32_t) floorf(xj)) + i;
				if(param->flags & LANCZOS_FLAG_EDGE_ZERO_PADDING)
				{
					// Zero Padding
					if((s1x < 0) || (s1x >= (param->src_w - 1)))
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
					else if(s1x >= param->src_w)
					{
						s1x = param->src_w - 1;
					}
				}

				lcoef = L((i - xj + floor(xj))/fs, param->a);
				sum  += s1[nch*s1x + ch]*lcoef;
				wj   += lcoef;
			}

			// Preserving Flux Normalization
			s2[nch*j + ch] = sum/wj;
		}
	}

	return 1;
}

static int
lanczos_resample_binningPass1D(lanczos_paramIrregular1D_t* param,
                               lanczos_irregularState_t* state)
{
	ASSERT(param);
	ASSERT(state);

	int32_t src_stride = 1 + param->channels;

	// binning pass
	int32_t i;
	int32_t ja;
	float   xi;
	float   jf;
	float   n2 = (float) param->dst_w;
	float   a  = (float) param->a;
	float   x0 = param->src_x0;
	float   x1 = param->src_x1;
	for(i = 0; i < param->src_count; ++i)
	{
		// compute xi2jf
		xi = param->src[src_stride*i + 0];
		jf = n2*(xi - x0)/(x1 - x0);

		// discard samples outside bin range
		// shift j to allow for support samples outside (x0..x1)
		ja = ((int32_t) floorf(jf)) + a;
		if((ja < 0) || (ja >= state->bin_count))
		{
			continue;
		}

		// store dat in list
		if(cc_list_append(state->bins[ja], NULL,
		                  &param->src[src_stride*i]) == NULL)
		{
			return 0;
		}
	}

	return 1;
}

static int
lanczos_resample_fillHole1D(lanczos_paramIrregular1D_t* param,
                            lanczos_irregularState_t* state,
                            int32_t ja)
{
	ASSERT(param);
	ASSERT(state);

	int32_t channels   = param->channels;
	int32_t src_stride = 1 + channels;

	float* dat = (float*) CALLOC(src_stride, sizeof(float));
	if(dat == NULL)
	{
		LOGE("CALLOC failed");
		return 0;
	}

	// compute jf2xi
	float x0 = param->src_x0;
	float x1 = param->src_x1;
	float n2 = param->dst_w;
	float jf = ((float) (ja - param->a)) + 0.5f;
	float xi = x0 + (x1 - x0)*jf/n2;
	dat[0]   = xi;

	cc_listIter_t* dat_iter;
	dat_iter = cc_list_append(state->holes, NULL, dat);
	if(dat_iter == NULL)
	{
		goto fail_holes;
	}

	if(cc_list_append(state->bins[ja], NULL, dat) == NULL)
	{
		goto fail_bins;
	}

	if(param->flags & LANCZOS_FLAG_NODATA_ZERO)
	{
		return 1;
	}

	// find the nearest sample in each direction
	cc_listIter_t* iter;
	int32_t jj;
	int32_t a   = param->a;
	float*  x0p = NULL;
	float*  x1p = NULL;
	float*  xp;
	for(jj = ja - 1; jj >= ja - a; --jj)
	{
		if(x0p || (jj < 0))
		{
			break;
		}

		iter = cc_list_head(state->bins[jj]);
		while(iter)
		{
			xp = (float*) cc_list_peekIter(iter);
			if((x0p == NULL) || (xp[0] > x0p[0]))
			{
				x0p = xp;
			}
			iter = cc_list_next(iter);
		}
	}
	for(jj = ja + 1; jj <= ja + a; ++jj)
	{
		if(x1p || (jj >= state->bin_count))
		{
			break;
		}

		iter = cc_list_head(state->bins[jj]);
		while(iter)
		{
			xp = (float*) cc_list_peekIter(iter);
			if((x1p == NULL) || (xp[0] < x1p[0]))
			{
				x1p = xp;
			}
			iter = cc_list_next(iter);
		}
	}

	// try LINEAR (default)
	int32_t ch;
	if(x0p && x1p &&
	   ((param->flags & LANCZOS_FLAG_NODATA_LINEAR) ||
	    ((param->flags & LANCZOS_FLAG_NODATA_MASK) == 0)))
	{
		float s = (xi - x0p[0])/(x1p[0] - x0p[0]);
		for(ch = 1; ch <= channels; ++ch)
		{
			dat[ch] = x0p[ch] + s*(x1p[ch] - x0p[ch]);
		}
		return 1;
	}

	// determine NEAREST and fallthrough to copy
	if(x0p && x1p && (fabs(x1p[0] - xi) < fabs(x0p[0] - xi)))
	{
		x0p = NULL;
	}

	// copy NEAREST or fallback to ZERO
	if(x0p)
	{
		memcpy(&dat[1], &x0p[1], channels*sizeof(float));
	}
	else if(x1p)
	{
		memcpy(&dat[1], &x1p[1], channels*sizeof(float));
	}

	// success
	return 1;

	// failure
	fail_bins:
		cc_list_remove(state->holes, &dat_iter);
	fail_holes:
		FREE(dat);
	return 0;
}

static int
lanczos_resample_holePass1D(lanczos_paramIrregular1D_t* param,
                            lanczos_irregularState_t* state)
{
	ASSERT(param);
	ASSERT(state);

	int32_t ja;
	for(ja = 0; ja < state->bin_count; ++ja)
	{
		if(cc_list_size(state->bins[ja]) > 0)
		{
			continue;
		}

		if(lanczos_resample_fillHole1D(param, state, ja) == 0)
		{
			return 0;
		}
	}

	return 1;
}

static int
lanczos_resample_resamplePass1D(lanczos_paramIrregular1D_t* param,
                                lanczos_irregularState_t* state)
{
	ASSERT(param);
	ASSERT(state);

	// TODO - lanczos_resample_resamplePass1D
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
			int32_t S  = param->dst_w/param->src_w;
			int32_t N  = S*2*param->a;
			int32_t fs = 1;
			return lanczos_resample_regular1DFast(param, S, N, fs);
		}
	}
	else if((param->src_w % param->dst_w) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		int32_t D  = param->src_w/param->dst_w;
		int32_t N  = D*2*param->a;
		int32_t fs = D;
		return lanczos_resample_regular1DFast(param, 1, N, fs);
	}

	// Arbitrary Resampling (Slow Path)
	return lanczos_resample_regular1DSlow(param);
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

	int32_t bin_count = param->dst_w + 2*param->a;

	lanczos_irregularState_t state = { 0 };
	if(lanczos_irregularState_init(&state, bin_count) == 0)
	{
		return 0;
	}

	if((lanczos_resample_binningPass1D(param, &state) == 0) ||
	   (lanczos_resample_holePass1D(param, &state) == 0)    ||
	   (lanczos_resample_resamplePass1D(param, &state) == 0))
	{
		goto failure;
	}

	lanczos_irregularState_discard(&state);

	// success
	return 1;

	// failure
	failure:
		lanczos_irregularState_discard(&state);
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
