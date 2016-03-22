#include "t_util.h"

#include <rgph_hash.h>
#include <stdlib.h> /* mi_vector_hash(3) on NetBSD. */
#include <string.h>

static const char msg[] =
	"Richard Of York Gave Battle In Vain.\n"
	"Ryanair Offers You Great Breaks In Venice.";

void
rgph_test_jenkins2(void)
{
	const uint8_t  u8[24]  = { 0x8, 1, 2, 3 };
	const uint16_t u16[24] = { 0x1661, 1, 2, 3 };
	const uint32_t u32[24] = { UINT32_C(0x32233223), 1, 2, 3 };
	const uint64_t u64[24] = { UINT64_C(0x6446644664466446), 1, 2, 3 };
	const float    f32[24] = { 32e23, 1, 2, 3 };
	const double   f64[24] = { 64e46, 1, 2, 3 };
	const uint32_t seed = 123456789;

	uint32_t h[12];
	size_t i;

	rgph_u32x3_jenkins2v_data(u8, sizeof(u8[0]), seed, h);
	rgph_u32x3_jenkins2v_u8(u8[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_u8a(u8, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(u8, sizeof(u8[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	rgph_u32x3_jenkins2v_data(u16, sizeof(u16[0]), seed, h);
	rgph_u32x3_jenkins2v_u16(u16[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_u16a(u16, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(u16, sizeof(u16[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	rgph_u32x3_jenkins2v_data(u32, sizeof(u32[0]), seed, h);
	rgph_u32x3_jenkins2v_u32(u32[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_u32a(u32, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(u32, sizeof(u32[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	rgph_u32x3_jenkins2v_data(u64, sizeof(u64[0]), seed, h);
	rgph_u32x3_jenkins2v_u64(u64[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_u64a(u64, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(u64, sizeof(u64[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	rgph_u32x3_jenkins2v_data(f32, sizeof(f32[0]), seed, h);
	rgph_u32x3_jenkins2v_f32(f32[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_f32a(f32, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(f32, sizeof(f32[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	rgph_u32x3_jenkins2v_data(f64, sizeof(f64[0]), seed, h);
	rgph_u32x3_jenkins2v_f64(f64[0], seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	rgph_u32x3_jenkins2v_f64a(f64, 1, seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#ifdef __NetBSD__
	mi_vector_hash(f64, sizeof(f64[0]), seed, h + 9);
	CHECK(h[0] == h[9]);
	CHECK(h[1] == h[10]);
	CHECK(h[2] == h[11]);
#endif
	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(u8, i * sizeof(u8[0]), seed, h);
		rgph_u32x3_jenkins2v_u8a(u8, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(u8, i * sizeof(u8[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(u16, i * sizeof(u16[0]), seed, h);
		rgph_u32x3_jenkins2v_u16a(u16, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(u16, i * sizeof(u16[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(u32, i * sizeof(u32[0]), seed, h);
		rgph_u32x3_jenkins2v_u32a(u32, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(u32, i * sizeof(u32[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(u64, i * sizeof(u64[0]), seed, h);
		rgph_u32x3_jenkins2v_u64a(u64, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(u64, i * sizeof(u64[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(f32, i * sizeof(f32[0]), seed, h);
		rgph_u32x3_jenkins2v_f32a(f32, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(f32, i * sizeof(f32[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	for (i = 0; i <= 24; i++) {
		rgph_u32x3_jenkins2v_data(f64, i * sizeof(f64[0]), seed, h);
		rgph_u32x3_jenkins2v_f64a(f64, i, seed, h + 4);
		CHECK(h[0] == h[4]);
		CHECK(h[1] == h[5]);
		CHECK(h[2] == h[6]);
#ifdef __NetBSD__
		mi_vector_hash(f64, i * sizeof(f64[0]), seed, h + 8);
		CHECK(h[0] == h[8]);
		CHECK(h[1] == h[9]);
		CHECK(h[2] == h[10]);
#endif
	}

	/* Test data of different lengths with different alignments. */
	for (i = 0; i < 4; i++) {
		uint32_t h[6];
		char *s;
		size_t l;

		for (l = 0; l <= sizeof(msg); l++) {

			s = malloc(i + (l + i > 0 ? l : 1));
			REQUIRE(s != NULL);
			memcpy(s + i, msg, l);

			rgph_u32x3_jenkins2v_data(msg, l, seed, &h[0]);
			rgph_u32x3_jenkins2v_data(s+i, l, seed, &h[3]);
			CHECK(h[0] == h[3]);
			CHECK(h[1] == h[4]);
			CHECK(h[2] == h[5]);

			free(s);
		}
	}
}
