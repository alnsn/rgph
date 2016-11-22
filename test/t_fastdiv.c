/*
 * Test some divisors to make sure that copypasting of NetBSD
 * and libdivide code didn't introduce any bugs.
 */
#include "t_util.h"

#include <rgph_fastdiv.h>

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static uint32_t
fastdiv_netbsd(uint32_t val, uint64_t mul, uint8_t s1, uint8_t s2)
{
	const uint32_t hi = (val * mul) >> 32;

	return (hi + ((val - hi) >> s1)) >> s2;
}

static uint32_t
fastdiv_libdivide(uint32_t val, uint64_t mul,
    uint8_t pre_shift, uint8_t post_shift, int increment)
{
	/*
	 *     if pre_shift > 0: emit("val >>>= pre_shift")
	 *     emit("result = mul * val")
	 *     if increment: emit("result = result + mul")
	 *     emit("result >>>= UINT_BITS")
	 *     if post_shift > 0: emit("result >>>= post_shift")
	 */
	return (mul * (val >> pre_shift) + mul * increment) >> 32 >> post_shift;
}

static void
test_fastdiv_netbsd(void)
{
	uint32_t div, mul;
	uint8_t s1, s2;

	div = 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 2;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 3;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 127;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 128;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 129;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = 137;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = INT32_MAX - 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div++; /* INT32_MAX */
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div++; /* INT32_MAX + 1 */
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = UINT32_MAX - 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);

	div = UINT32_MAX;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 0, NULL);
	CHECK(fastdiv_netbsd(0, mul, s1, s2) == 0 / div);
	CHECK(fastdiv_netbsd(1, mul, s1, s2) == 1 / div);
	CHECK(fastdiv_netbsd(div, mul, s1, s2) == div / div);
	CHECK(fastdiv_netbsd(1234, mul, s1, s2) == 1234 / div);
	CHECK(fastdiv_netbsd(SHRT_MAX, mul, s1, s2) == SHRT_MAX / div);
	CHECK(fastdiv_netbsd(USHRT_MAX, mul, s1, s2) == USHRT_MAX / div);
	CHECK(fastdiv_netbsd(123456789, mul, s1, s2) == 123456789 / div);
	CHECK(fastdiv_netbsd(INT32_MAX, mul, s1, s2) == INT32_MAX / div);
	CHECK(fastdiv_netbsd(UINT32_MAX, mul, s1, s2) == UINT32_MAX / div);
}

static void
test_fastdiv_libdivide_31(void)
{
	uint32_t div, mul;
	uint8_t s1, s2;
	int inc;

	div = 3;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);

	div = 127;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);

	div = 129;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);

	div = 137;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);

	div = INT32_MAX - 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);

	div = INT32_MAX;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 31, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
}

static void
test_fastdiv_libdivide_32(void)
{
	uint32_t div, mul;
	uint8_t s1, s2;
	int inc;

	div = 3;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = 127;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = 129;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = 137;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = INT32_MAX - 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = INT32_MAX;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = UINT32_MAX - 1;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);

	div = UINT32_MAX;
	rgph_fastdiv_prepare(div, &mul, &s1, &s2, 32, &inc);
	CHECK(fastdiv_libdivide(0, mul, s1, s2, inc) == 0 / div);
	CHECK(fastdiv_libdivide(1, mul, s1, s2, inc) == 1 / div);
	CHECK(fastdiv_libdivide(div, mul, s1, s2, inc) == div / div);
	CHECK(fastdiv_libdivide(1234, mul, s1, s2, inc) == 1234 / div);
	CHECK(fastdiv_libdivide(SHRT_MAX, mul, s1, s2, inc) == SHRT_MAX / div);
	CHECK(fastdiv_libdivide(USHRT_MAX, mul, s1, s2, inc) == USHRT_MAX / div);
	CHECK(fastdiv_libdivide(123456789, mul, s1, s2, inc) == 123456789 / div);
	CHECK(fastdiv_libdivide(INT32_MAX, mul, s1, s2, inc) == INT32_MAX / div);
	CHECK(fastdiv_libdivide(UINT32_MAX, mul, s1, s2, inc) == UINT32_MAX / div);
}

void
rgph_test_fastdiv(void)
{

	test_fastdiv_netbsd();
	test_fastdiv_libdivide_31();
	test_fastdiv_libdivide_32();
}
