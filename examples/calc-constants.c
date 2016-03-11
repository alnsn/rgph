/*
 * Calculate constants MAX_NKEYS_R2, MAX_NKEYS_R3, MAX_FASTDIV_R2
 * and MAX_FASTDIV_R3 in ../lib/graph.cc and rank2_max.fastdiv and
 * rank3_max.fastdiv in ../test/graph.lua.
 *
 * gcc -O -Wall -I../lib -Llib -lrgph examples/calc-range.c
 */

#include "rgph_fastdiv.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static size_t
round_up(size_t n, size_t r)
{

	return n > -r ? 0 : (n + (r - 1)) / r * r;
}

static bool
is_fastdiv(uint32_t n, size_t nbits)
{
	uint32_t mul;
	uint8_t s1 = 0, s2;
	int inc = 0;

	if (n & (n - 1))
		rgph_fastdiv_prepare(n, &mul, &s1, &s2, nbits, &inc);

	return s1 == 0 && inc == 0;
}

static uint32_t
graph_nverts(size_t nkeys, int r)
{
	uint32_t nverts = (r == 2) ? 2 * nkeys + (nkeys + 7) / 8
	                           : 1 * nkeys + (nkeys + 3) / 4;
	nverts = round_up(nverts, r);
	return nverts;
}

static uint32_t
calc_max_nkeys(int r)
{
	const double c = (r == 2) ? 2.125 : 1.25;
	uint32_t n = (UINT32_MAX - r) / c;
	uint32_t u, v;

	u = graph_nverts(n++, r);
	while ((v = graph_nverts(n++, r)) > u)
		u = v;

	return n - 2;
}

int main(int argc, char *argv[])
{
	const size_t nbits = 32;
	const int r = argc > 1 ? argv[1][0] - '0' : 2;
	const double c = (r == 2) ? 2.125 : 1.25;

	if (r < 2 || r > 3) {
		fprintf(stderr, "rank can only be 2 or 3");
		return EXIT_FAILURE;
	}

	const size_t max_nkeys = calc_max_nkeys(r);
	const size_t max_nverts = graph_nverts(max_nkeys, r);
	size_t nkeys, fastdiv = max_nverts / r;

	assert(max_nverts > max_nkeys && max_nverts == fastdiv * r);

	while (!is_fastdiv(fastdiv, nbits))
		fastdiv--;

	nkeys = (fastdiv - 1) * r / c;
	while (nkeys <= max_nkeys && graph_nverts(nkeys, r) <= fastdiv * r)
		nkeys++;
	nkeys--;

	printf("#define MAX_NKEYS_R%d 0x%08zx\n", r, max_nkeys);
	printf("#define MAX_FASTDIV_R%d 0x%08zx\n", r, fastdiv);
	printf("local rank%d_max = { fastdiv=0x%08zx }\n", r, nkeys);

	return EXIT_SUCCESS;
}
