#include "t_util.h"

#include <rgph_hash.h>
#include <string.h>

static const char msg[] =
	"Richard Of York Gave Battle In Vain.\n"
	"Ryanair Offers You Great Breaks In Venice.";


static void
rgph_test_murmur32s_x4(void)
{
	const uint8_t  u8[24]  = { 0x8, 1, 2, 3 };
	const uint16_t u16[24] = { 0x1661, 1, 2, 3 };
	const uint32_t u32[24] = { UINT32_C(0x42233223), 1, 2, 3 };
	const uint64_t u64[24] = { UINT64_C(0x6446644664466446), 1, 2, 3 };
	const float    f32[24] = { 32e23, 1, 2, 3 };
	const double   f64[24] = { 64e46, 1, 2, 3 };
	const uint32_t seed = 123456789;

	uint8_t h[12];
	char *s;
	size_t i, l;

	rgph_u8x4_murmur32s_data(u8, sizeof(u8[0]), seed, h);
	rgph_u8x4_murmur32s_u8(u8[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_u8a(u8, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	rgph_u8x4_murmur32s_data(u16, sizeof(u16[0]), seed, h);
	rgph_u8x4_murmur32s_u16(u16[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_u16a(u16, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	rgph_u8x4_murmur32s_data(u32, sizeof(u32[0]), seed, h);
	rgph_u8x4_murmur32s_u32(u32[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_u32a(u32, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	rgph_u8x4_murmur32s_data(u64, sizeof(u64[0]), seed, h);
	rgph_u8x4_murmur32s_u64(u64[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_u64a(u64, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	rgph_u8x4_murmur32s_data(f32, sizeof(f32[0]), seed, h);
	rgph_u8x4_murmur32s_f32(f32[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_f32a(f32, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	rgph_u8x4_murmur32s_data(f64, sizeof(f64[0]), seed, h);
	rgph_u8x4_murmur32s_f64(f64[0], seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);
	CHECK(h[2] == h[6]);
	CHECK(h[3] == h[7]);
	rgph_u8x4_murmur32s_f64a(f64, 1, seed, h + 8);
	CHECK(h[0] == h[8]);
	CHECK(h[1] == h[9]);
	CHECK(h[2] == h[10]);
	CHECK(h[3] == h[11]);

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(u8, i * sizeof(u8[0]), seed, h);
		rgph_u8x4_murmur32s_u8a(u8, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(u16, i * sizeof(u16[0]), seed, h);
		rgph_u8x4_murmur32s_u16a(u16, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(u32, i * sizeof(u32[0]), seed, h);
		rgph_u8x4_murmur32s_u32a(u32, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(u64, i * sizeof(u64[0]), seed, h);
		rgph_u8x4_murmur32s_u64a(u64, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(f32, i * sizeof(f32[0]), seed, h);
		rgph_u8x4_murmur32s_f32a(f32, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u8x4_murmur32s_data(f64, i * sizeof(f64[0]), seed, h);
		rgph_u8x4_murmur32s_f64a(f64, i, seed, h + 5);
		CHECK(h[0] == h[5]);
		CHECK(h[1] == h[6]);
		CHECK(h[2] == h[7]);
		CHECK(h[3] == h[8]);
	}

	/* Test data of different length with different alignment. */
	s = malloc(sizeof(msg) + 4);
	REQUIRE(s != NULL);

	for (i = 0; i < 4; i++) {
		memcpy(s + i, msg, sizeof(msg));
		for (l = 0; l <= sizeof(msg); l++) {
			uint8_t h[8];
			rgph_u8x4_murmur32s_data(msg, l, seed, &h[0]);
			rgph_u8x4_murmur32s_data(s+i, l, seed, &h[4]);
			CHECK(h[0] == h[4]);
			CHECK(h[1] == h[5]);
			CHECK(h[2] == h[6]);
			CHECK(h[3] == h[7]);
			CHECK(rgph_u32_murmur32s_data(msg, l, seed) ==
			      rgph_u32_murmur32s_data(s+i, l, seed));
		}
	}

	free(s);
}

static void
rgph_test_murmur32s_x2(void)
{
	const uint8_t  u8[24]  = { 0x8, 1, 2, 3 };
	const uint16_t u16[24] = { 0x1661, 1, 2, 3 };
	const uint32_t u32[24] = { UINT32_C(0x22233223), 1, 2, 3 };
	const uint64_t u64[24] = { UINT64_C(0x6446644664466446), 1, 2, 3 };
	const float    f32[24] = { 32e23, 1, 2, 3 };
	const double   f64[24] = { 64e46, 1, 2, 3 };
	const uint32_t seed = 123456789;

	uint16_t h[6];
	char *s;
	size_t i, l;

	rgph_u16x2_murmur32s_data(u8, sizeof(u8[0]), seed, h);
	rgph_u16x2_murmur32s_u8(u8[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_u8a(u8, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	rgph_u16x2_murmur32s_data(u16, sizeof(u16[0]), seed, h);
	rgph_u16x2_murmur32s_u16(u16[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_u16a(u16, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	rgph_u16x2_murmur32s_data(u32, sizeof(u32[0]), seed, h);
	rgph_u16x2_murmur32s_u32(u32[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_u32a(u32, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	rgph_u16x2_murmur32s_data(u64, sizeof(u64[0]), seed, h);
	rgph_u16x2_murmur32s_u64(u64[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_u64a(u64, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	rgph_u16x2_murmur32s_data(f32, sizeof(f32[0]), seed, h);
	rgph_u16x2_murmur32s_f32(f32[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_f32a(f32, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	rgph_u16x2_murmur32s_data(f64, sizeof(f64[0]), seed, h);
	rgph_u16x2_murmur32s_f64(f64[0], seed, h + 2);
	CHECK(h[0] == h[2]);
	CHECK(h[1] == h[3]);
	rgph_u16x2_murmur32s_f64a(f64, 1, seed, h + 4);
	CHECK(h[0] == h[4]);
	CHECK(h[1] == h[5]);

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(u8, i * sizeof(u8[0]), seed, h);
		rgph_u16x2_murmur32s_u8a(u8, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(u16, i * sizeof(u16[0]), seed, h);
		rgph_u16x2_murmur32s_u16a(u16, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(u32, i * sizeof(u32[0]), seed, h);
		rgph_u16x2_murmur32s_u32a(u32, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(u64, i * sizeof(u64[0]), seed, h);
		rgph_u16x2_murmur32s_u64a(u64, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(f32, i * sizeof(f32[0]), seed, h);
		rgph_u16x2_murmur32s_f32a(f32, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	for (i = 0; i <= 24; i++) {
		rgph_u16x2_murmur32s_data(f64, i * sizeof(f64[0]), seed, h);
		rgph_u16x2_murmur32s_f64a(f64, i, seed, h + 3);
		CHECK(h[0] == h[3]);
		CHECK(h[1] == h[4]);
	}

	/* Test data of different length with different alignment. */
	s = malloc(sizeof(msg) + 4);
	REQUIRE(s != NULL);

	for (i = 0; i < 4; i++) {
		memcpy(s + i, msg, sizeof(msg));
		for (l = 0; l <= sizeof(msg); l++) {
			uint16_t h[4];
			rgph_u16x2_murmur32s_data(msg, l, seed, &h[0]);
			rgph_u16x2_murmur32s_data(s+i, l, seed, &h[2]);
			CHECK(h[0] == h[2]);
			CHECK(h[1] == h[3]);
			CHECK(rgph_u32_murmur32s_data(msg, l, seed) ==
			      rgph_u32_murmur32s_data(s+i, l, seed));
		}
	}

	free(s);
}

void
rgph_test_murmur32s(void)
{

	rgph_test_murmur32s_x4();
	rgph_test_murmur32s_x2();
}
