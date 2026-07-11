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
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "lanczos"
#include "libcc/rng/cc_rngUniform.h"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "liblanczos/lanczos_irregular1D.h"

/***********************************************************
* public                                                   *
***********************************************************/

int main(int argc, const char** argv)
{
	if(argc != 4)
	{
		LOGI("usage: %s a s1_count n2", argv[0]);
		return EXIT_FAILURE;
	}

	cc_rngUniform_t rng;
	cc_rngUniform_init(&rng);

	lanczos_irregular1D_t param =
	{
		.flags    = 0,
		.a        = (int32_t) strtol(argv[1], NULL, 0),
		.channels = 1,
		.s1_count = (int32_t) strtol(argv[2], NULL, 0),
		.x0       = 0.0f,
		.x1       = 2.0f*M_PI,
		.n2       = (int32_t) strtol(argv[3], NULL, 0),
	};

	param.s1 = (float*)
	           CALLOC(2*param.s1_count, sizeof(float));
	if(param.s1 == NULL)
	{
		LOGE("CALLOC failed");
		return EXIT_FAILURE;
	}

	param.s2 = (float*)
	           CALLOC(param.n2, sizeof(float));
	if(param.s2 == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_s2;
	}

	// generate s1 including irregular support samples
	float   x;
	float   y;
	int32_t i;
	float   n2     = (float) param.n2;
	float   a1     = (float) param.a;
	float   a2     = 2.0f*a1;
	float   range  = 2.0f*M_PI*(n2 + a2)/n2;
	float   offset = 2.0f*M_PI*a1/n2;
	for(i = 0; i < param.s1_count; ++i)
	{
		x = range*cc_rngUniform_rand1F(&rng) - offset;
		y = sinf(x);

		param.s1[2*i + 0] = x;
		param.s1[2*i + 1] = y;
	}

	char s1_dat[256];
	char s2_dat[256];
	snprintf(s1_dat, 256, "irregular-sine-%u-%u.dat",
	         param.a, param.s1_count);
	snprintf(s2_dat, 256, "irregular-sine-%u-%u-%u.dat",
	         param.a, param.s1_count, param.n2);

	// open data files
	FILE* fs1_dat = fopen(s1_dat, "w");
	if(fs1_dat == NULL)
	{
		LOGE("fopen %s failed", s1_dat);
		goto fail_fs1_dat;
	}

	FILE* fs2_dat = fopen(s2_dat, "w");
	if(fs2_dat == NULL)
	{
		LOGE("fopen %s failed", s2_dat);
		goto fail_fs2_dat;
	}

	// resample data
	if(lanczos_irregular1D_resample(&param) == 0)
	{
		goto fail_resample;
	}

	// export s1 data
	for(i = 0; i < param.s1_count; ++i)
	{
		fprintf(fs1_dat, "%f %f\n",
		        param.s1[2*i + 0],
		        param.s1[2*i + 1]);
	}

	// export s2 data
	int32_t j;
	float   xj;
	float   x0 = param.x0;
	float   x1 = param.x1;
	for(j = 0; j < param.n2; ++j)
	{
		xj = x0 + (x1 - x0)*(j + 0.5f)/n2;
		fprintf(fs2_dat, "%f %f\n", xj, param.s2[j]);
	}

	fclose(fs2_dat);
	fclose(fs1_dat);
	FREE(param.s2);
	FREE(param.s1);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_resample:
		fclose(fs2_dat);
	fail_fs2_dat:
		fclose(fs1_dat);
	fail_fs1_dat:
		FREE(param.s2);
	fail_s2:
		FREE(param.s1);
	return EXIT_FAILURE;
}
