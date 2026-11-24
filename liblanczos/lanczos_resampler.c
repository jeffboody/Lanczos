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

#include <float.h>
#include <math.h>
#include <stdlib.h>

#define LOG_TAG "lanczos"
#include "../../libcc/cc_log.h"
#include "../../libcc/cc_memory.h"
#include "lanczos_resampler.h"

/*
 * private lanczos_kernel
 */

static float sinc(float x)
{
	if(x == 0.0f)
	{
		return 1.0f;
	}

	return sinf(M_PI*x)/(M_PI*x);
}

static float lanczos_kernel(float x, float a)
{
	if((-a < x) && (x < a))
	{
		return sinc(x)*sinc(x/a);
	}

	return 0.0f;
}

/*
 * private
 */

static int
lanczos_resampler_resizeLcoef(lanczos_resampler_t* self,
                              int32_t N)
{
	ASSERT(self);

	if(N <= 0)
	{
		LOGE("invalid N=%i", N);
		return 0;
	}

	size_t size = N*sizeof(float);
	if(size <= MEMSIZEPTR(self->lcoef))
	{
		return 1;
	}

	float* lcoef = (float*) REALLOC(self->lcoef, size);
	if(lcoef == NULL)
	{
		LOGE("REALLOC failed");
		return 0;
	}
	self->lcoef = lcoef;

	return 1;
}

static int
lanczos_resampler_resizeWeights(lanczos_resampler_t* self,
                                int32_t phases)
{
	ASSERT(self);

	if(phases <= 0)
	{
		LOGE("invalid phases=%i", phases);
		return 0;
	}

	size_t size = phases*sizeof(float);
	if(size <= MEMSIZEPTR(self->w))
	{
		return 1;
	}

	float* w = (float*) REALLOC(self->w, size);
	if(w == NULL)
	{
		LOGE("REALLOC failed");
		return 0;
	}
	self->w = w;

	return 1;
}

static int
lanczos_resampler_applyFastX(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2,
                             int32_t phases, int32_t N,
                             int32_t fs)
{
	ASSERT(self);
	ASSERT(param);
	ASSERT(s1);
	ASSERT(s2);

	if((lanczos_resampler_resizeLcoef(self, N) == 0) ||
	   (lanczos_resampler_resizeWeights(self, phases) == 0))
	{
		return 0;
	}

	float* lcoef = self->lcoef;
	float* w     = self->w;
	float  a     = param->a;

	float xi;
	float step = ((float) s1->width)/((float) s2->width);

	// precompute Lanczos kernel coefficients and
	// normalizing weights
	int32_t m;
	int32_t ii;
	int32_t idx = 0;
	int32_t m0  = -((int32_t) fs*a) + 1;
	int32_t m1  = (int32_t) fs*a;
	for(ii = 0; ii < phases; ++ii)
	{
		xi    = (ii + 0.5f)*step - 0.5f;
		w[ii] = 0.0f;
		for(m = m0; m <= m1; ++m)
		{
			if(idx >= N)
			{
				LOGE("invalid idx=%m, N=%u", idx, N);
				return 0;
			}

			lcoef[idx] = lanczos_kernel((m - xi + floorf(xi))/fs, a);
			w[ii]     += lcoef[idx];
			++idx;
		}
	}

	if(idx != N)
	{
		LOGE("invalid idx=%i, N=%u", idx, N);
		return 0;
	}

	// commpute s2[i2,j,k]
	int32_t i1;
	int32_t i2;
	int32_t j;
	int32_t k;
	int32_t ch;
	int32_t channels = s1->channels;
	float   sum;
	for(ch = 0; ch < channels; ++ch)
	{
		// reset idx
		idx = 0;

		for(j = 0; j < s2->height; ++j)
		{
			for(k = 0; k < s2->depth; ++k)
			{
				for(i2 = 0; i2 < s2->width; ++i2)
				{
					sum = 0.0f;
					ii  = i2%phases;
					xi  = (i2 + 0.5f)*step - 0.5f;
					for(m = m0; m <= m1; ++m)
					{
						// Edge Handling
						i1 = ((int32_t) floorf(xi)) + m;
						if(param->edge_mode == LANCZOS_EDGE_MODE_ZERO)
						{
							// Zero Padding
							if((i1 < 0) || (i1 >= s1->width))
							{
								++idx;
								continue;
							}
						}
						else
						{
							// Clamping
							if(i1 < 0)
							{
								i1 = 0;
							}
							else if(i1 >= s1->width)
							{
								i1 = s1->width - 1;
							}
						}

						sum += lanczos_signal_get(s1, ch, i1, j, k)*
						       lcoef[idx%N];
						++idx;
					}

					// Preserving Flux Normalization
					lanczos_signal_set(s2, ch, i2, j, k, sum/w[ii]);
				}
			}
		}
	}

	return 1;
}

static int
lanczos_resampler_applyFastY(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2,
                             int32_t phases, int32_t N,
                             int32_t fs)
{
	// TODO - lanczos_resampler_applyFastY
	return 0;
}

static int
lanczos_resampler_applyFastZ(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2,
                             int32_t phases, int32_t N,
                             int32_t fs)
{
	// TODO - lanczos_resampler_applyFastZ
	return 0;
}

static int
lanczos_resampler_applySlowX(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2)
{
	ASSERT(self);

	float fs = 1.0f;
	if(s2->width < s1->width)
	{
		fs = ((float) s1->width)/((float) s2->width);
	}

	float step = ((float) s1->width)/((float) s2->width);
	float a    = param->a;

	// commpute s2[i2,j,k]
	int32_t ch;
	int32_t channels = s1->channels;
	int32_t i1;
	int32_t i2;
	int32_t j;
	int32_t k;
	int32_t m;
	int32_t m0;
	int32_t m1;
	float   sum;
	float   xi;
	float   w;
	float   lcoef;
	for(ch = 0; ch < channels; ++ch)
	{
		for(j = 0; j < s2->height; ++j)
		{
			for(k = 0; k < s2->depth; ++k)
			{
				for(i2 = 0; i2 < s2->width; ++i2)
				{
					sum = 0.0f;
					w   = 0.0f;
					xi  = (i2 + 0.5f)*step - 0.5f;
					m0 = (int32_t) floorf(-fs*a + 1 + (xi - floorf(xi)));
					m1 = (int32_t) floorf(fs*a + (xi - floorf(xi)));
					for(m = m0; m <= m1; ++m)
					{
						// Edge Handling
						i1 = ((int32_t) floorf(xi)) + m;
						if(param->edge_mode == LANCZOS_EDGE_MODE_ZERO)
						{
							// Zero Padding
							if((i1 < 0) || (i1 >= s1->width))
							{
								continue;
							}
						}
						else
						{
							// Clamping
							if(i1 < 0)
							{
								i1 = 0;
							}
							else if(i1 >= s1->width)
							{
								i1 = s1->width - 1;
							}
						}

						lcoef = lanczos_kernel((m - xi + floor(xi))/fs, a);
						sum += lanczos_signal_get(s1, ch, i1, j, k)*lcoef;
						w   += lcoef;
					}

					// Preserving Flux Normalization
					lanczos_signal_set(s2, ch, i2, j, k, sum/w);
				}
			}
		}
	}

	return 1;
}

static int
lanczos_resampler_applySlowY(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2)
{
	// TODO - lanczos_resampler_applySlowY
	return 0;
}

static int
lanczos_resampler_applySlowZ(lanczos_resampler_t* self,
                             lanczos_param_t* param,
                             lanczos_signal_t* s1,
                             lanczos_signal_t* s2)
{
	// TODO - lanczos_resampler_applySlowZ
	return 0;
}

static int
lanczos_resampler_applyX(lanczos_resampler_t* self,
                         lanczos_param_t* param,
                         lanczos_signal_t* s1,
                         lanczos_signal_t* s2)
{
	ASSERT(self);
	ASSERT(param);
	ASSERT(s1);
	ASSERT(s2);

	if(lanczos_signal_resize(s2) == 0)
	{
		return 0;
	}

	// detect resampling fast paths
	if(s2->width >= s1->width)
	{
		if((s2->width % s1->width) == 0)
		{
			// Upsampling (Fast Path)
			// Resampling Factor: S = n2/n1
			// Total Coefficients: N = S*2*a
			// Filter Scale: 1
			int32_t S  = s2->width/s1->width;
			int32_t N  = S*2*param->a;
			int32_t fs = 1;
			return lanczos_resampler_applyFastX(self, param, s1, s2,
			                                    S, N, fs);
		}
	}
	else if((s1->width % s2->width) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		int32_t D  = s1->width/s2->width;
		int32_t N  = D*2*param->a;
		int32_t fs = D;
		return lanczos_resampler_applyFastX(self, param, s1, s2,
		                                    1, N, fs);
	}

	// Arbitrary Resampling (Slow Path)
	return lanczos_resampler_applySlowX(self, param, s1, s2);
}

static int
lanczos_resampler_applyY(lanczos_resampler_t* self,
                         lanczos_param_t* param,
                         lanczos_signal_t* s1,
                         lanczos_signal_t* s2)
{
	ASSERT(self);
	ASSERT(param);
	ASSERT(s1);
	ASSERT(s2);

	if(lanczos_signal_resize(s2) == 0)
	{
		return 0;
	}

	// detect resampling fast paths
	if(s2->width >= s1->width)
	{
		if((s2->width % s1->width) == 0)
		{
			// Upsampling (Fast Path)
			// Resampling Factor: S = n2/n1
			// Total Coefficients: N = S*2*a
			// Filter Scale: 1
			int32_t S  = s2->width/s1->width;
			int32_t N  = S*2*param->a;
			int32_t fs = 1;
			return lanczos_resampler_applyFastY(self, param, s1, s2,
			                                    S, N, fs);
		}
	}
	else if((s1->width % s2->width) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		int32_t D  = s1->width/s2->width;
		int32_t N  = D*2*param->a;
		int32_t fs = D;
		return lanczos_resampler_applyFastY(self, param, s1, s2,
		                                    1, N, fs);
	}

	// Arbitrary Resampling (Slow Path)
	return lanczos_resampler_applySlowY(self, param, s1, s2);
}

static int
lanczos_resampler_applyZ(lanczos_resampler_t* self,
                         lanczos_param_t* param,
                         lanczos_signal_t* s1,
                         lanczos_signal_t* s2)
{
	ASSERT(self);
	ASSERT(param);
	ASSERT(s1);
	ASSERT(s2);

	if(lanczos_signal_resize(s2) == 0)
	{
		return 0;
	}

	// detect resampling fast paths
	if(s2->width >= s1->width)
	{
		if((s2->width % s1->width) == 0)
		{
			// Upsampling (Fast Path)
			// Resampling Factor: S = n2/n1
			// Total Coefficients: N = S*2*a
			// Filter Scale: 1
			int32_t S  = s2->width/s1->width;
			int32_t N  = S*2*param->a;
			int32_t fs = 1;
			return lanczos_resampler_applyFastZ(self, param, s1, s2,
			                                    S, N, fs);
		}
	}
	else if((s1->width % s2->width) == 0)
	{
		// Downsampling (Fast Path)
		// Resampling Factor: S = 1/D = n2/n1
		// Total Coefficients: N = D*2*a
		// Filter Scale: fs = n1/n2
		int32_t D  = s1->width/s2->width;
		int32_t N  = D*2*param->a;
		int32_t fs = D;
		return lanczos_resampler_applyFastZ(self, param, s1, s2,
		                                    1, N, fs);
	}

	// Arbitrary Resampling (Slow Path)
	return lanczos_resampler_applySlowZ(self, param, s1, s2);
}

/*
 * public
 */

lanczos_resampler_t* lanczos_resampler_new(void)
{
	lanczos_resampler_t* self;
	self = (lanczos_resampler_t*)
	       CALLOC(1, sizeof(lanczos_resampler_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	return self;
}

void lanczos_resampler_delete(lanczos_resampler_t** _self)
{
	ASSERT(_self);

	lanczos_resampler_t* self = *_self;
	if(self)
	{
		lanczos_resampler_discard(self);
		FREE(self);
		*_self = NULL;
	}
}

void lanczos_resampler_discard(lanczos_resampler_t* self)
{
	ASSERT(self);

	lanczos_signal_discard(&self->sy);
	lanczos_signal_discard(&self->sx);

	FREE(self->w);
	self->w = NULL;

	FREE(self->lcoef);
	self->lcoef = NULL;
}

int lanczos_resampler_apply(lanczos_resampler_t* self,
                            lanczos_param_t* param,
                            lanczos_signal_t* s1,
                            lanczos_signal_t* s2)
{
	ASSERT(self);
	ASSERT(param);
	ASSERT(s1);
	ASSERT(s2);

	size_t sz1 = lanczos_signal_sizeBytes(s1);
	size_t sz2 = lanczos_signal_sizeBytes(s2);

	// validate input
	// s2->data may be NULL
	if(param->a == 0)
	{
		LOGE("invalid a=%u", param->a);
		return 0;
	}
	else if((sz1 == 0) || (MEMSIZEPTR(s1->data) < sz1))
	{
		LOGE("invalid s1: channels=%u, width=%u, height=%u, depth=%u, data=%p",
		     s1->channels, s1->width, s1->height, s1->depth,
		     s1->data);
		return 0;
	}
	else if(sz2 == 0)
	{
		LOGE("invalid s2: channels=%u, width=%u, height=%u, depth=%u",
		     s2->channels, s2->width, s2->height, s2->depth);
		return 0;
	}
	else if(s1->channels != s2->channels)
	{
		LOGE("invalid channels=%u:%u",
		     s1->channels, s2->channels);
		return 0;
	}

	// count passes
	uint32_t passes = 0;
	passes += (s1->width  == s2->width)  ? 0 : 1;
	passes += (s1->height == s2->height) ? 0 : 1;
	passes += (s1->depth  == s2->depth)  ? 0 : 1;

	// identity transformation
	if(passes == 0)
	{
		return lanczos_signal_copy(s1, s2);
	}

	// apply X pass
	lanczos_signal_t* sprev = s1;
	lanczos_signal_t* snext;
	if(s1->width != s2->width)
	{
		// apply final pass
		if(passes == 1)
		{
			return lanczos_resampler_applyX(self, param, sprev, s2);
		}

		// apply intermediate pass
		snext = &self->sx;
		snext->channels = sprev->channels;
		snext->width    = s2->width;
		snext->height   = sprev->height;
		snext->depth    = sprev->depth;
		if(lanczos_resampler_applyX(self, param, sprev, snext) == 0)
		{
			return 0;
		}

		// finish pass
		sprev = snext;
		--passes;
	}

	// apply Y pass
	if(s1->height != s2->height)
	{
		// apply final pass
		if(passes == 1)
		{
			return lanczos_resampler_applyY(self, param, sprev, s2);
		}

		// apply intermediate pass
		snext = &self->sy;
		snext->channels = sprev->channels;
		snext->width    = sprev->width;
		snext->height   = s2->height;
		snext->depth    = sprev->depth;
		if(lanczos_resampler_applyY(self, param, sprev, snext) == 0)
		{
			return 0;
		}

		// finish pass
		sprev = snext;
		--passes;
	}

	// apply Z pass
	return lanczos_resampler_applyZ(self, param, sprev, s2);
}
