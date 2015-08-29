/*-
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * Copyright (c) 2015 Alexander Nasonov.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "hash_priv.h"
#include "rgph_hash.h"

#include <stddef.h>
#include <stdint.h>

inline void
rgph_u32x4_murmur32_u8(uint8_t value, uint32_t seed, uint32_t *h)
{
	const uint32_t c1 = RGPH_MURMUR32_MUL1;
	const uint32_t c2 = RGPH_MURMUR32_MUL2;

	uint32_t k1 = 0;

	h[0] = h[1] = h[2] = h[3] = seed;

	k1 ^= value;
	k1 *= c1; k1 = rgph_rotl(k1, 15); k1 *= c2; h[0] ^= k1;

	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u16(uint16_t value, uint32_t seed, uint32_t *h)
{
	const uint32_t c1 = RGPH_MURMUR32_MUL1;
	const uint32_t c2 = RGPH_MURMUR32_MUL2;

	uint32_t k1 = 0;

	h[0] = h[1] = h[2] = h[3] = seed;

	k1 ^= htole16(value);
	k1 *= c1; k1 = rgph_rotl(k1, 15); k1 *= c2; h[0] ^= k1;

	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u32(uint32_t value, uint32_t seed, uint32_t *h)
{
	const uint32_t c1 = RGPH_MURMUR32_MUL1;
	const uint32_t c2 = RGPH_MURMUR32_MUL2;

	uint32_t k1 = 0;

	h[0] = h[1] = h[2] = h[3] = seed;

	k1 ^= htole32(value);
	k1 *= c1; k1 = rgph_rotl(k1, 15); k1 *= c2; h[0] ^= k1;

	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u64(uint64_t value, uint32_t seed, uint32_t *h)
{
	const uint32_t c1 = RGPH_MURMUR32_MUL1;
	const uint32_t c2 = RGPH_MURMUR32_MUL2;
	const uint32_t c3 = RGPH_MURMUR32_MUL3;

	uint32_t k1 = 0;
	uint32_t k2 = 0;

	h[0] = h[1] = h[2] = h[3] = seed;

	k2 ^= htole64(value) >> 32;
        k2 *= c2; k2 = rgph_rotl(k2, 16); k2 *= c3; h[1] ^= k2;

	k1 ^= htole64(value) & UINT32_MAX;
	k1 *= c1; k1 = rgph_rotl(k1, 15); k1 *= c2; h[0] ^= k1;

	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_f32(float value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_u32(f2u32(value), seed, h);
}

inline void
rgph_u32x4_murmur32_f64(double value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_u64(d2u64(value), seed, h);
}

uint32_t
rgph_u32_murmur32_u8(uint8_t value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_u8(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_murmur32_u16(uint16_t value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_u16(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_murmur32_u32(uint32_t value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_u32(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_murmur32_u64(uint64_t value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_u64(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_murmur32_f32(float value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_f32(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_murmur32_f64(double value, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_f64(value, seed, h);
	return h[0];
}

uint64_t
rgph_u64_murmur32_u8(uint8_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_u8(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_murmur32_u16(uint16_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_u16(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_murmur32_u32(uint32_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_u32(value, seed, h);
	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_murmur32_u64(uint64_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_u64(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_murmur32_f32(float value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_f32(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_murmur32_f64(double value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_f64(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

inline void
rgph_u32x4_murmur32_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint32_t c1 = RGPH_MURMUR32_MUL1;
	const uint32_t c2 = RGPH_MURMUR32_MUL2;
	const uint32_t c3 = RGPH_MURMUR32_MUL3;
	const uint32_t c4 = RGPH_MURMUR32_MUL4;

	const uint8_t *end = key + len;
	uint32_t k1, k2, k3, k4;
#if defined(RGPH_UNALIGNED_READ)
	const int down = 0;
#else
	uint32_t w[4];
	const int down = ((uintptr_t)key) & 3;
	uint32_t carry = len > 0 && down != 0 ? rgph_read32a(key - down) : 0;
#endif

	h[0] = h[1] = h[2] = h[3] = seed;

	if (down == 0) {
		for (; end - key >= 16; key += 16) {
			k1 = rgph_read32a(&key[0]);
			k2 = rgph_read32a(&key[4]);
			k3 = rgph_read32a(&key[8]);
			k4 = rgph_read32a(&key[12]);
			RGPH_MURMUR32_MIX(k1, k2, k3, k4, h);
		}
	} else {
#if !defined(RGPH_UNALIGNED_READ)
		for (; end - key >= 16; key += 16) {
			rgph_read32u(key, 4 - down, &carry, w, 4);
			k1 = w[0];
			k2 = w[1];
			k3 = w[2];
			k4 = w[3];
			RGPH_MURMUR32_MIX(k1, k2, k3, k4, h);
		}
#endif
	}

	k1 = k2 = k3 = k4 = 0;

	switch ((end - key) & 15) {
	case 15: k4 ^= key[14] << 16;
	case 14: k4 ^= key[13] << 8;
	case 13: k4 ^= key[12];
	         k4 *= c4; k4  = rgph_rotl(k4, 18); k4 *= c1; h[3] ^= k4;
		 /* FALLTHROUGH */
	case 12: k3 ^= key[11] << 24;
	case 11: k3 ^= key[10] << 16;
	case 10: k3 ^= key[9] << 8;
	case  9: k3 ^= key[8];
	         k3 *= c3; k3  = rgph_rotl(k3, 17); k3 *= c4; h[2] ^= k3;
		 /* FALLTHROUGH */
	case  8: k2 ^= key[7] << 24;
	case  7: k2 ^= key[6] << 16;
	case  6: k2 ^= key[5] << 8;
	case  5: k2 ^= key[4];
	         k2 *= c2; k2  = rgph_rotl(k2, 16); k2 *= c3; h[1] ^= k2;
		 /* FALLTHROUGH */
	case  4: k1 ^= key[3] << 24;
	case  3: k1 ^= key[2] << 16;
	case  2: k1 ^= key[1] << 8;
	case  1: k1 ^= key[0];
	         k1 *= c1; k1  = rgph_rotl(k1, 15); k1 *= c2; h[0] ^= k1;
	};

	rgph_murmur32_finalise(len, h);
}

inline void
rgph_u32x4_murmur32_data(const void * restrict data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;

	rgph_u32x4_murmur32_u8a(key, len, seed, h);
}

uint32_t
rgph_u32_murmur32_data(const void * restrict data,
    size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint32_t h[4];

	rgph_u32x4_murmur32_u8a(key, len, seed, h);
	return h[0];
}

uint64_t
rgph_u64_murmur32_data(const void * restrict data,
    size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_u8a(key, len, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

void
rgph_u32x4_murmur32_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	return rgph_u32x4_murmur32_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}

void
rgph_u32x4_murmur32_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	/* XXX implement */
	return rgph_u32x4_murmur32_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}

void
rgph_u32x4_murmur32_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	/* XXX implement */
	return rgph_u32x4_murmur32_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}

void
rgph_u32x4_murmur32_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	/* XXX implement */
	return rgph_u32x4_murmur32_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}

void
rgph_u32x4_murmur32_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	/* XXX implement */
	return rgph_u32x4_murmur32_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}
