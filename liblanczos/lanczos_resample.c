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

#include <stdlib.h>

#define LOG_TAG "lanczos"
#include "../../libcc/cc_log.h"
#include "lanczos_resample.h"

/*
 * public
 */

int lanczos_resample_regular1D(lanczos_paramRegular1D_t* param)
{
	ASSERT(param);
	ASSERT(param->src);
	ASSERT(param->dst);

	// TODO - lanczos_resample_regular1D
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
