/*
 * Calculate constants MAX_NKEYS_R2_XXX, MAX_NKEYS_R3_XXX,
 * MAX_FASTDIV_R2_XXX and MAX_FASTDIV_R3_XXX in ../lib/graph.cc and
 * constants in rank2_max and rank3_max tables in ../test/graph.lua.
 *
 * gcc -O -Wall -Ilib -Llib -lrgph examples/calc-constants.c
 */

#include "rgph_fastdiv.h"

#include <assert.h>
#include <limits.h>
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
	int inc = 1; /* Power of 2 isn't fastdiv. */

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

/*
 * If hash_bits is big enough, nverts is limited by uint32_t modulo
 * arithmetic. Otherwise, max nverts is determined by a number of
 * bits available to each graph partition. Partition size can be up
 * to P = (1u << partition_bits) but values in range (P/2, P) give
 * non-uniform distribution. We take away one bit to smoothen
 * a distribution. XXX Is one bit enough?
 */
static uint32_t
estimate_max_nverts(int hash_bits, int r)
{
	const size_t partition_bits = hash_bits / r;

	return (partition_bits >= sizeof(uint32_t) * CHAR_BIT) ?
	    UINT32_MAX : ((1u << (partition_bits - 1))) * r;
}

static uint32_t
calc_max_nkeys(int hash_bits, int r)
{
	const size_t max_nverts = estimate_max_nverts(hash_bits, r);
	const double c = (r == 2) ? 2.125 : 1.25;
	uint32_t n = (max_nverts - r) / c;

	if (max_nverts < UINT32_MAX) {
		while (graph_nverts(n, r) <= max_nverts)
			n++;
		return n - 1;
	} else {
		uint32_t u, v;

		u = graph_nverts(n++, r);
		while ((v = graph_nverts(n++, r)) > u)
			u = v;
		return n - 2;
	}
}

int main(int argc, char *argv[])
{
	const long r = argc > 1 ? strtol(argv[1], NULL, 10) : 2;
	const double c = (r == 2) ? 2.125 : 1.25;
	const long hash_bits = argc > 2 ? strtol(argv[2], NULL, 10) : 128;

	if (hash_bits != 32 && hash_bits != 64 &&
	    hash_bits != 96 && hash_bits != 128) {
		fprintf(stderr, "wrong number of hash bits (argument #2)");
		return EXIT_FAILURE;
	}

	if (r < 2 || r > 3) {
		fprintf(stderr, "rank can only be 2 or 3 (argument #1)");
		return EXIT_FAILURE;
	}

	const bool full_range = (hash_bits >= r * 32);
	const size_t div_nbits = sizeof(uint32_t) * CHAR_BIT - !full_range;
	const size_t max_nkeys = calc_max_nkeys(hash_bits, r);
	const size_t max_nverts = graph_nverts(max_nkeys, r);
	size_t nkeys, fastdiv = max_nverts / r;

	assert(max_nverts > max_nkeys && max_nverts == fastdiv * r);

	while (!is_fastdiv(fastdiv, div_nbits))
		fastdiv--;

	nkeys = (fastdiv - 1) * r / c;
	while (nkeys <= max_nkeys && graph_nverts(nkeys, r) <= fastdiv * r)
		nkeys++;
	nkeys--;

	printf("max fastdiv       0x%08zx\n", fastdiv);
	printf("max fastdiv nkeys 0x%08zx\n", nkeys);
	printf("max nkeys         0x%08zx\n", max_nkeys);
	printf("max nverts        0x%08zx\n", max_nverts);

	return EXIT_SUCCESS;
}
