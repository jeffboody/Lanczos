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
#include "liblanczos/lanczos_resample.h"

/***********************************************************
* public                                                   *
***********************************************************/

int main(int argc, const char** argv)
{
	if(argc != 4)
	{
		LOGI("usage: %s a src_w dst_w", argv[0]);
		return EXIT_FAILURE;
	}

	lanczos_paramRegular1D_t param =
	{
		.flags    = 0,
		.a        = (int32_t) strtol(argv[1], NULL, 0),
		.channels = 1,
		.src_w    = (int32_t) strtol(argv[2], NULL, 0),
		.dst_w    = (int32_t) strtol(argv[3], NULL, 0),
	};

	param.src = (float*) CALLOC(param.src_w, sizeof(float));
	if(param.src == NULL)
	{
		LOGE("CALLOC failed");
		return EXIT_FAILURE;
	}

	param.dst = (float*) CALLOC(param.dst_w, sizeof(float));
	if(param.dst == NULL)
	{
		LOGE("CALLOC failed");
		goto fail_dst;
	}

	// generate src data
	float   a;
	float   b = (float) (param.src_w - 1);
	int32_t i;
	for(i = 0; i < param.src_w; ++i)
	{
		a = (float) i;
		param.src[i] = sinf(2.0f*M_PI*a/b);
	}

	char src_dat[256];
	char dst_dat[256];
	snprintf(src_dat, 256, "sine-%u-%u.dat",
	         param.a, param.src_w);
	snprintf(dst_dat, 256, "sine-%u-%u-%u.dat",
	         param.a, param.src_w, param.dst_w);

	// open data files
	FILE* fsrc_dat = fopen(src_dat, "w");
	if(fsrc_dat == NULL)
	{
		LOGE("fopen %s failed", src_dat);
		goto fail_fsrc_dat;
	}

	FILE* fdst_dat = fopen(dst_dat, "w");
	if(fdst_dat == NULL)
	{
		LOGE("fopen %s failed", dst_dat);
		goto fail_fdst_dat;
	}

	// resample data
	if(lanczos_resample_regular1D(&param) == 0)
	{
		goto fail_resample;
	}

	// export src data
	for(i = 0; i < param.src_w; ++i)
	{
		fprintf(fsrc_dat, "%f %f\n", (float) i, param.src[i]);
	}

	// export dst data
	int32_t j;
	float   xj;
	float   step = ((float) param.src_w)/
	               ((float) param.dst_w);
	for(j = 0; j < param.dst_w; ++j)
	{
		xj = (j + 0.5f)*step - 0.5f;
		fprintf(fdst_dat, "%f %f\n", xj, param.dst[j]);
	}

	fclose(fdst_dat);
	fclose(fsrc_dat);
	FREE(param.dst);
	FREE(param.src);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_resample:
		fclose(fdst_dat);
	fail_fdst_dat:
		fclose(fsrc_dat);
	fail_fsrc_dat:
		FREE(param.dst);
	fail_dst:
		FREE(param.src);
	return EXIT_FAILURE;
}
