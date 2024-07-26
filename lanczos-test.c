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

// clamp-to-edge outside bounds or
// zero outside bounds
#define LANCZOS_USE_CLAMP_TO_EDGE

/***********************************************************
* private                                                  *
***********************************************************/

static double sinc(double x)
{
	if(x == 0.0)
	{
		return 1.0;
	}

	return sin(M_PI*x)/(M_PI*x);
}

static double L(double x, double a)
{
	if((-a < x) && (x < a))
	{
		return sinc(x)*sinc(x/a);
	}

	return 0.0;
}

#define N1 10

static double S1(int i)
{
	double s1[N1] =
	{
		0.1, 0.3, 0.4, 0.3, 0.2,
		0.4, 0.6, 0.8, 0.9, 0.7,
	};

	#ifdef LANCZOS_USE_CLAMP_TO_EDGE
		// clamp-to-edge outside bounds
		if(i < 0)
		{
			i = 0;
		}
		else if(i >= 10)
		{
			i = 9;
		}
	#else
		// zero outside bounds
		if((i < 0) || (i >= 10))
		{
			return 0.0;
		}
	#endif

	return s1[i];
}

/***********************************************************
* public                                                   *
***********************************************************/

int main(int argc, char** argv)
{
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

	// export s1
	int    n1 = N1;
	double s1;
	int i;
	for(i = 0; i < n1; ++i)
	{
		s1 = S1(i);
		fprintf(fs1, "%lf %lf\n", (double) i, s1);
		fprintf(fs1, "%lf %lf\n", (double) (i + 1), s1);
	}

	// downsample
	int    j;
	int    n2 = 5;
	int    fs = n1/n2;
	int    a  = 3;
	double fsd = (double) fs;
	double s2;
	double l;
	double x;
	double w;
	double jd;
	double step = ((double) n1)/((double) n2);
	printf("downsample n1=%i, n2=%i\n", n1, n2);
	for(j = 0; j < n2; ++j)
	{
		jd = (double) j;
		x  = (jd + 0.5)*step - 0.5;
		w  = 0.0;
		s2 = 0.0;
		printf("j=%i, x=%lf\n", j, x);
		for(i = -(fs*a) + 1; i <= (fs*a); ++i)
		{
			l   = L((i - x + floor(x))/fsd, a);
			s1  = S1(floor(x) + i);
			printf("i=%i, L(%lf)=%lf, S1(%lf)=%f\n",
			       i,
			       (double) (i - x + floor(x))/fsd, l,
			       (double) (floor(x) + i), s1);
			s2 += s1*l;
			w  += l;
		}
		printf("s2=%lf, s2/w=%lf, w=%lf\n", s2, s2/w, w);
		fprintf(fs2d, "%lf %lf %lf\n", x, s2/w, w);
	}
	printf("\n");

	// upsample
	n2   = 20;
	a    = 3;
	step = ((double) n1)/((double) n2);
	printf("upsample n1=%i, n2=%i\n", n1, n2);
	for(j = 0; j < n2; ++j)
	{
		jd = (double) j;
		x  = (jd + 0.5)*step - 0.5;
		w  = 0.0;
		s2 = 0.0;
		printf("j=%i, x=%lf\n", j, x);
		for(i = -a + 1; i <= a; ++i)
		{
			l   = L(i - x + floor(x), a);
			s1  = S1(floor(x) + i);
			printf("i=%i, L(%lf)=%lf, S1(%lf)=%f\n",
			       i,
			       (double) (i - x + floor(x)), l,
			       (double) (floor(x) + i), s1);
			s2 += s1*l;
			w  += l;
		}
		printf("s2=%lf, s2/w=%lf, w=%lf\n", s2, s2/w, w);
		fprintf(fs2u, "%lf %lf %lf\n", x, s2/w, w);
	}

	fclose(fs2u);
	fclose(fs2d);
	fclose(fs1);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_fs2u:
		fclose(fs2d);
	fail_fs2d:
		fclose(fs1);
	return EXIT_FAILURE;
}
