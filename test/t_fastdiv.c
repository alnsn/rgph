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

void
rgph_test_fastdiv(void)
{

	test_fastdiv_netbsd();
}
