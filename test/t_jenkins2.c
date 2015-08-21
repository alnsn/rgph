#include "t_util.h"

#include <rgph_hash.h>
#include <stdlib.h> /* mi_vector_hash(3) on NetBSD */

void rgph_test_jenkins2(void)
{
	const uint8_t u8 = 0x8;
	const uint16_t u16 = 0x1616;
	const uint32_t u32 = UINT32_C(0x32323232);
	const uint64_t u64 = UINT64_C(0x6464646464646464);
	const uint32_t seed = 123456789;
	const float f32 = 32e32;
	const double f64 = 64e64;

	uint32_t h[9];
	uint64_t h64;

	rgph_u32x3_jenkins2_data(&u8, sizeof(u8), seed, h);
	rgph_u32x3_jenkins2_u8(u8, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_u8(u8, seed) == h[0]);
#ifdef __NetBSD__
	mi_vector_hash(&u8, sizeof(u8), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_data(&u8, sizeof(u8), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	h64 = rgph_u64_jenkins2_u8(u8, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	rgph_u32x3_jenkins2_data(&u16, sizeof(u16), seed, h);
	rgph_u32x3_jenkins2_u16(u16, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_u16(u16, seed) == h[0]);
#ifdef __NetBSD__
	h64 = rgph_u64_jenkins2_data(&u16, sizeof(u16), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	mi_vector_hash(&u16, sizeof(u16), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_u16(u16, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	rgph_u32x3_jenkins2_data(&u32, sizeof(u32), seed, h);
	rgph_u32x3_jenkins2_u32(u32, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_u32(u32, seed) == h[0]);
#ifdef __NetBSD__
	mi_vector_hash(&u32, sizeof(u32), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_data(&u32, sizeof(u32), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	h64 = rgph_u64_jenkins2_u32(u32, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	rgph_u32x3_jenkins2_data(&u64, sizeof(u64), seed, h);
	rgph_u32x3_jenkins2_u64(u64, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_u64(u64, seed) == h[0]);
#ifdef __NetBSD__
	mi_vector_hash(&u64, sizeof(u64), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_data(&u64, sizeof(u64), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	h64 = rgph_u64_jenkins2_u64(u64, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	rgph_u32x3_jenkins2_data(&f32, sizeof(f32), seed, h);
	rgph_u32x3_jenkins2_f32(f32, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_f32(f32, seed) == h[0]);
#ifdef __NetBSD__
	mi_vector_hash(&f32, sizeof(f32), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_data(&f32, sizeof(f32), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	h64 = rgph_u64_jenkins2_f32(f32, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	rgph_u32x3_jenkins2_data(&f64, sizeof(f64), seed, h);
	rgph_u32x3_jenkins2_f64(f64, seed, h + 3);
	CHECK(h[0] == h[3]);
	CHECK(h[1] == h[4]);
	CHECK(h[2] == h[5]);
	CHECK(rgph_u32_jenkins2_f64(f64, seed) == h[0]);
#ifdef __NetBSD__
	mi_vector_hash(&f64, sizeof(f64), seed, h + 6);
	CHECK(h[0] == h[6]);
	CHECK(h[1] == h[7]);
	CHECK(h[2] == h[8]);
#endif
	h64 = rgph_u64_jenkins2_data(&f64, sizeof(f64), seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);
	h64 = rgph_u64_jenkins2_f64(f64, seed);
	CHECK((h64 & 0xffffffff) == h[0]);
	CHECK((h64 >> 32) == h[1]);

	/* XXX test hashes for arrays and strings of different lengths. */
}
