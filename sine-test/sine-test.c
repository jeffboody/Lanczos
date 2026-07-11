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
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "liblanczos/lanczos_regular1D.h"

/***********************************************************
* public                                                   *
***********************************************************/

int main(int argc, const char** argv)
{
	if(argc != 4)
	{
		LOGI("usage: %s a n1 n2", argv[0]);
		return EXIT_FAILURE;
	}

	lanczos_regular1D_t param =
	{
		.flags    = 0,
		.a        = (int32_t) strtol(argv[1], NULL, 0),
		.channels = 1,
		.n1       = (int32_t) strtol(argv[2], NULL, 0),
		.n2       = (int32_t) strtol(argv[3], NULL, 0),
	};

	param.s1 = (float*) CALLOC(param.n1, sizeof(float));
	if(param.s1 == NULL)
	{
		LOGE("CALLOC failed");
		return EXIT_FAILURE;
	}

	param.s2 = (float*) CALLOC(param.n2, sizeof(float));
	if(param.s2 == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_dst;
	}

	// generate s1 data
	float   a;
	float   b = (float) (param.n1 - 1);
	int32_t i;
	for(i = 0; i < param.n1; ++i)
	{
		a = (float) i;
		param.s1[i] = sinf(2.0f*M_PI*a/b);
	}

	char s1_dat[256];
	char s2_dat[256];
	snprintf(s1_dat, 256, "sine-%u-%u.dat",
	         param.a, param.n1);
	snprintf(s2_dat, 256, "sine-%u-%u-%u.dat",
	         param.a, param.n1, param.n2);

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
	if(lanczos_regular1D_resample(&param) == 0)
	{
		goto fail_resample;
	}

	// export s1 data
	for(i = 0; i < param.n1; ++i)
	{
		fprintf(fs1_dat, "%f %f\n", (float) i, param.s1[i]);
	}

	// export s2 data
	int32_t j;
	float   xj;
	float   step = ((float) param.n1)/
	               ((float) param.n2);
	for(j = 0; j < param.n2; ++j)
	{
		xj = (j + 0.5f)*step - 0.5f;
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
	fail_dst:
		FREE(param.s1);
	return EXIT_FAILURE;
}
