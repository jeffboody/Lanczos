/*
 * Copyright (c) 2024 Jeff Boody
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
#include "../libcc/cc_log.h"
#include "liblanczos/lanczos_resample.h"

/***********************************************************
* public                                                   *
***********************************************************/

int main(int argc, char** argv)
{
	// open data files
	FILE* fs1 = fopen("s1.dat", "w");
	if(fs1 == NULL)
	{
		printf("fopen s1.dat failed\n");
		return EXIT_FAILURE;
	}

	FILE* fs2d = fopen("s2d.dat", "w");
	if(fs2d == NULL)
	{
		printf("fopen s2d.dat failed\n");
		goto fail_fs2d;
	}

	FILE* fs2u = fopen("s2u.dat", "w");
	if(fs2u == NULL)
	{
		printf("fopen s2u.dat failed\n");
		goto fail_fs2u;
	}

	float src[10] =
	{
		0.1f, 0.3f, 0.4f, 0.3f, 0.2f,
		0.4f, 0.6f, 0.8f, 0.9f, 0.7f,
	};

	// initialize parameters
	float dstu[20];
	float dstd[5];
	lanczos_paramRegular1D_t paramu =
	{
		.flags    = 0,
		.a        = 3,
		.channels = 1,
		.src_w    = sizeof(src)/sizeof(float),
		.dst_w    = sizeof(dstu)/sizeof(float),
		.src      = src,
		.dst      = dstu,
	};

	lanczos_paramRegular1D_t paramd =
	{
		.flags    = paramu.flags,
		.a        = paramu.a,
		.channels = paramu.channels,
		.src_w    = paramu.src_w,
		.dst_w    = sizeof(dstd)/sizeof(float),
		.src      = src,
		.dst      = dstd,
	};

	// resample data
	if((lanczos_resample_regular1D(&paramu) == 0) ||
	   (lanczos_resample_regular1D(&paramd) == 0))
	{
		goto fail_resample;
	}

	// export s1 data
	uint32_t i;
	for(i = 0; i < paramu.src_w; ++i)
	{
		fprintf(fs1, "%f %f\n", (float) i, src[i]);
	}

	// export s2u data
	uint32_t j;
	float    xj;
	float    step = ((float) paramu.src_w)/
	                ((float) paramu.dst_w);
	for(j = 0; j < paramu.dst_w; ++j)
	{
		xj = (j + 0.5f)*step - 0.5f;
		fprintf(fs2u, "%f %f\n", xj, dstu[j]);
	}

	// export s2d data
	step = ((float) paramd.src_w)/
	       ((float) paramd.dst_w);
	for(j = 0; j < paramd.dst_w; ++j)
	{
		xj = (j + 0.5f)*step - 0.5f;
		fprintf(fs2d, "%f %f\n", xj, dstd[j]);
	}

	fclose(fs2u);
	fclose(fs2d);
	fclose(fs1);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_resample:
		fclose(fs2u);
	fail_fs2u:
		fclose(fs2d);
	fail_fs2d:
		fclose(fs1);
	return EXIT_FAILURE;
}
