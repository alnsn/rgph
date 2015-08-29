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

inline uint32_t
rgph_u32_murmur32s_u8(uint8_t value, uint32_t seed)
{
	uint32_t h[1] = { seed };

	rgph_murmur32s_mix(value, h, 1);
	rgph_murmur32s_finalise(sizeof(value), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_u16(uint16_t value, uint32_t seed)
{
	uint32_t h[1] = { seed };

	rgph_murmur32s_mix(htole16(value), h, 1);
	rgph_murmur32s_finalise(sizeof(value), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_u32(uint32_t value, uint32_t seed)
{
	uint32_t h[1] = { seed };

	rgph_murmur32s_mix(htole32(value), h, 0);
	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(sizeof(value), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_u64(uint64_t value, uint32_t seed)
{
	uint32_t h[1] = { seed };

	rgph_murmur32s_mix(htole64(value) & UINT32_MAX, h, 0);
	rgph_murmur32s_mix(htole64(value) >> 32, h, 0);
	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(sizeof(value), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_f32(float value, uint32_t seed)
{

	return rgph_u32_murmur32s_u32(f2u32(value), seed);
}

inline uint32_t
rgph_u32_murmur32s_f64(double value, uint32_t seed)
{

	return rgph_u32_murmur32s_u64(d2u64(value), seed);
}

inline uint32_t
rgph_u32_murmur32s_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed)
{
	const uint8_t *end = key + len;
	uint32_t k, h[1] = { seed };
#if defined(RGPH_UNALIGNED_READ)
	const int down = 0;
#else
	uint32_t w[4];
	const int down = ((uintptr_t)key) & 3;
	uint32_t carry = len > 0 && down != 0 ? rgph_read32a(key - down) : 0;
#endif

	if (down == 0) {
		for (; end - key >= 4; key += 4)
			rgph_murmur32s_mix(rgph_read32a(&key[0]), h, 0);
	} else {
#if !defined(RGPH_UNALIGNED_READ)
		for (; end - key >= 16; key += 16) {
			rgph_read32u(key, 4 - down, &carry, w, 4);
			rgph_murmur32s_mix(w[0], h, 0);
			rgph_murmur32s_mix(w[1], h, 0);
			rgph_murmur32s_mix(w[2], h, 0);
			rgph_murmur32s_mix(w[3], h, 0);
		}
		for (; end - key >= 4; key += 4) {
			rgph_read32u(key, 4 - down, &carry, w, 1);
			rgph_murmur32s_mix(w[0], h, 0);
		}
#endif
	}

	k = 0;
	switch (end - key) {
	case 3:
		k ^= (uint32_t)key[2] << 16; /* FALLTHROUGH */
	case 2:
		k ^= (uint32_t)key[1] << 8;  /* FALLTHROUGH */
	case 1:
		k ^= (uint32_t)key[0];       /* FALLTHROUGH */
	case 0:
		rgph_murmur32s_mix(k, h, 1);
		rgph_murmur32s_finalise(len, h);
	}

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_data(const void * restrict data,
    size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;

	return rgph_u32_murmur32s_u8a(key, len, seed);
}

inline uint32_t
rgph_u32_murmur32s_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data((const void *)(const char *)key,
	    len * sizeof(key[0]), seed);
}

inline uint32_t
rgph_u32_murmur32s_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed)
{
	const uint32_t *end = key + len;
	uint32_t h[1] = { seed };

	for (; key != end; key += 1)
		rgph_murmur32s_mix(htole32(key[0]), h, 0);

	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(len * sizeof(key[0]), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed)
{
	const uint64_t *end = key + len;
	uint32_t h[1] = { seed };

	for (; key != end; key += 1) {
		rgph_murmur32s_mix(htole64(key[0]) & UINT32_MAX, h, 0);
		rgph_murmur32s_mix(htole64(key[0]) >> 32, h, 0);
	}

	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(len * sizeof(key[0]), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_f32a(const float * restrict key,
    size_t len, uint32_t seed)
{
	const float *end = key + len;
	uint32_t h[1] = { seed };

	for (; key != end; key += 1)
		rgph_murmur32s_mix(htole32(f2u32(key[0])), h, 0);

	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(len * sizeof(key[0]), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_f64a(const double * restrict key,
    size_t len, uint32_t seed)
{
	const double *end = key + len;
	uint32_t h[1] = { seed };

	for (; key != end; key += 1) {
		rgph_murmur32s_mix(htole64(d2u64(key[0])) & UINT32_MAX, h, 0);
		rgph_murmur32s_mix(htole64(d2u64(key[0])) >> 32, h, 0);
	}

	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(len * sizeof(key[0]), h);

	return h[0];
}

void
rgph_u8x4_murmur32s_u8(uint8_t value, uint32_t seed, uint8_t *h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u8(value, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_u16(uint16_t value, uint32_t seed, uint8_t *h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u16(value, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

inline void
rgph_u8x4_murmur32s_u32(uint32_t value, uint32_t seed, uint8_t *h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u32(value, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

inline void
rgph_u8x4_murmur32s_u64(uint64_t value, uint32_t seed, uint8_t *h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u64(value, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_f32(float value, uint32_t seed, uint8_t *h8)
{

	rgph_u8x4_murmur32s_u32(f2u32(value), seed, h8);
}

void
rgph_u8x4_murmur32s_f64(double value, uint32_t seed, uint8_t *h8)
{

	rgph_u8x4_murmur32s_u64(d2u64(value), seed, h8);
}

void
rgph_u8x4_murmur32s_data(const void * restrict data,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint32_t h;

	h = rgph_u32_murmur32s_data(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u8a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u16a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u32a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u64a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_f32a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u8x4_murmur32s_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_f64a(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u16x2_murmur32s_u8(uint8_t value, uint32_t seed, uint16_t *h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u8(value, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_u16(uint16_t value, uint32_t seed, uint16_t *h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u16(value, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

inline void
rgph_u16x2_murmur32s_u32(uint32_t value, uint32_t seed, uint16_t *h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u32(value, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

inline void
rgph_u16x2_murmur32s_u64(uint64_t value, uint32_t seed, uint16_t *h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u64(value, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_f32(float value, uint32_t seed, uint16_t *h16)
{

	rgph_u16x2_murmur32s_u32(f2u32(value), seed, h16);
}

void
rgph_u16x2_murmur32s_f64(double value, uint32_t seed, uint16_t *h16)
{

	rgph_u16x2_murmur32s_u64(d2u64(value), seed, h16);
}

void
rgph_u16x2_murmur32s_data(const void * restrict data,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint32_t h;

	h = rgph_u32_murmur32s_u8a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u8a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u16a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u32a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_u64a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_f32a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

void
rgph_u16x2_murmur32s_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_f64a(key, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}
