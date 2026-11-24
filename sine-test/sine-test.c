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
#include "liblanczos/lanczos_resampler.h"

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

	lanczos_resampler_t resampler = { 0 };

	lanczos_param_t param =
	{
		.a          = (uint32_t) strtol(argv[1], NULL, 0),
		.edge_mode  = LANCZOS_EDGE_MODE_CLAMP,
		.clamp_mode = LANCZOS_CLAMP_MODE_ENABLE,
		.clamp_min  = -1.0f,
		.clamp_max  = 1.0f,
	};

	uint32_t channels = 1;
	uint32_t n1       = (uint32_t) strtol(argv[2], NULL, 0);
	uint32_t n2       = (uint32_t) strtol(argv[3], NULL, 0);

	lanczos_signal_t* s1;
	s1 = lanczos_signal_new(channels, n1, 1, 1);
	if(s1 == NULL)
	{
		goto fail_s1;
	}

	lanczos_signal_t* s2;
	s2 = lanczos_signal_new(channels, n2, 1, 1);
	if(s2 == NULL)
	{
		goto fail_s2;
	}

	// generate s1 data
	float   a;
	float   b = (float) (n1 - 1);
	int32_t i;
	uint32_t ch = 0;
	for(i = 0; i < n1; ++i)
	{
		a = (float) i;
		lanczos_signal_set(s1, ch, i, 0, 0, sinf(2.0f*M_PI*a/b));
	}

	char s1_name[256];
	char s2_name[256];
	snprintf(s1_name, 256, "sine-%u-%u.dat",
	         param.a, n1);
	snprintf(s2_name, 256, "sine-%u-%u-%u.dat",
	         param.a, n1, n2);

	// open data files
	FILE* fs1 = fopen(s1_name, "w");
	if(fs1 == NULL)
	{
		LOGE("fopen %s failed", s1_name);
		goto fail_fs1;
	}

	FILE* fs2 = fopen(s2_name, "w");
	if(fs2 == NULL)
	{
		LOGE("fopen %s failed", s2_name);
		goto fail_fs2;
	}

	// resample data
	if(lanczos_resampler_apply(&resampler, &param, s1, s2) == 0)
	{
		goto fail_resample;
	}

	// export s1 data
	for(i = 0; i < n1; ++i)
	{
		fprintf(fs1, "%f %f\n", (float) i,
		        lanczos_signal_get(s1, ch, i, 0, 0));
	}

	// export s2 data
	float x;
	float step = ((float) n1)/((float) n2);
	for(i = 0; i < n2; ++i)
	{
		x = (i + 0.5f)*step - 0.5f;
		fprintf(fs2, "%f %f\n", (float) x,
		        lanczos_signal_get(s2, ch, i, 0, 0));
	}

	fclose(fs2);
	fclose(fs1);
	lanczos_signal_delete(&s2);
	lanczos_signal_delete(&s1);
	lanczos_resampler_discard(&resampler);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_resample:
		fclose(fs2);
	fail_fs2:
		fclose(fs1);
	fail_fs1:
		lanczos_signal_delete(&s2);
	fail_s2:
		lanczos_signal_delete(&s1);
	fail_s1:
		lanczos_resampler_discard(&resampler);
	return EXIT_FAILURE;
}
