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
#include "lanczos_irregular1D.h"
#include "lanczos_kernel.h"

/*
 * private irregularState
 */

typedef struct
{
	// bins contain pointers to s1 or hole elements
	// which covers the s2 samples and support window
	int32_t     bin_count; // bin_count = n2 + 2*a
	cc_list_t** bins;      // size = bin_count
	cc_list_t*  holes;     // element_size = 1 + channels
} lanczos_irregularState_t;

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
			cc_list_discard(state->bins[i]);
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

/*
 * private
 */

static float
lanczos_irregular1D_mapjf(lanczos_irregular1D_t* self,
                          float xi)
{
	ASSERT(self);

	float n2 = (float) self->n2;
	float x0 = self->x0;
	float x1 = self->x1;
	return n2*(xi - x0)/(x1 - x0);
}

static float
lanczos_irregular1D_invjf(lanczos_irregular1D_t* self,
                          float jf)
{
	ASSERT(self);

	float n2 = (float) self->n2;
	float x0 = self->x0;
	float x1 = self->x1;
	return x0 + (x1 - x0)*jf/n2;
}

static int
lanczos_irregular1D_binningPass(lanczos_irregular1D_t* self,
                                lanczos_irregularState_t* state)
{
	ASSERT(self);
	ASSERT(state);

	int32_t s1_stride = 1 + self->channels;

	// binning pass
	int32_t i;
	int32_t ja;
	float   xi;
	float   jf;
	float   a  = (float) self->a;
	for(i = 0; i < self->s1_count; ++i)
	{
		// compute jf
		xi = self->s1[s1_stride*i + 0];
		jf = lanczos_irregular1D_mapjf(self, xi);

		// discard samples outside bin range
		// shift j to allow for support samples outside (x0..x1)
		ja = ((int32_t) floorf(jf)) + a;
		if((ja < 0) || (ja >= state->bin_count))
		{
			continue;
		}

		// store dat in list
		if(cc_list_append(state->bins[ja], NULL,
		                  &self->s1[s1_stride*i]) == NULL)
		{
			return 0;
		}
	}

	return 1;
}

static int
lanczos_irregular1D_fillHole(lanczos_irregular1D_t* self,
                             lanczos_irregularState_t* state,
                             int32_t ja)
{
	ASSERT(self);
	ASSERT(state);

	int32_t channels  = self->channels;
	int32_t s1_stride = 1 + channels;

	float* dat = (float*) CALLOC(s1_stride, sizeof(float));
	if(dat == NULL)
	{
		LOGE("CALLOC failed");
		return 0;
	}

	// compute xi
	float jf = ((float) (ja - self->a)) + 0.5f;
	float xi = lanczos_irregular1D_invjf(self, jf);
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

	if(self->flags & LANCZOS_FLAG_NODATA_ZERO)
	{
		return 1;
	}

	// find the nearest sample in each direction
	cc_listIter_t* iter;
	int32_t jj;
	int32_t a   = self->a;
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
	   ((self->flags & LANCZOS_FLAG_NODATA_LINEAR) ||
	    ((self->flags & LANCZOS_FLAG_NODATA_MASK) == 0)))
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
lanczos_irregular1D_holePass(lanczos_irregular1D_t* self,
                             lanczos_irregularState_t* state)
{
	ASSERT(self);
	ASSERT(state);

	int32_t ja;
	for(ja = 0; ja < state->bin_count; ++ja)
	{
		if(cc_list_size(state->bins[ja]) > 0)
		{
			continue;
		}

		if(lanczos_irregular1D_fillHole(self, state, ja) == 0)
		{
			return 0;
		}
	}

	return 1;
}

static int
lanczos_irregular1D_resamplePass(lanczos_irregular1D_t* self,
                                 lanczos_irregularState_t* state)
{
	ASSERT(self);
	ASSERT(state);

	// TODO - lanczos_irregular1D_resamplePass
	return 1;
}

/*
 * public
 */

int
lanczos_irregular1D_resample(lanczos_irregular1D_t* self)
{
	ASSERT(self);

	int32_t bin_count = self->n2 + 2*self->a;

	lanczos_irregularState_t state = { 0 };
	if(lanczos_irregularState_init(&state, bin_count) == 0)
	{
		return 0;
	}

	if((lanczos_irregular1D_binningPass(self, &state) == 0) ||
	   (lanczos_irregular1D_holePass(self, &state) == 0)    ||
	   (lanczos_irregular1D_resamplePass(self, &state) == 0))
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
