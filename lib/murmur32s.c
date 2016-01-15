/*-
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * Copyright (c) 2015-2016 Alexander Nasonov.
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

 /*
  * XXX use rgph_unalias.
  */

#include "rgph_hash_impl.h"
#include "rgph_hash.h"

#include <stddef.h>
#include <stdint.h>

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
/* Non-public external symbols for aliasing. */
uint32_t rgph_u32_murmur32s_data32(const void *, size_t, uint32_t);
uint32_t rgph_u32_murmur32s_data64(const void *, size_t, uint32_t);
void rgph_u8x4_murmur32s_data32(const void *, size_t, uint32_t, uint8_t *);
void rgph_u8x4_murmur32s_data64(const void *, size_t, uint32_t, uint8_t *);
void rgph_u16x2_murmur32s_data32(const void *, size_t, uint32_t, uint16_t *);
void rgph_u16x2_murmur32s_data64(const void *, size_t, uint32_t, uint16_t *);
#endif

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

	return rgph_u32_murmur32s_u32(rgph_f2u32(value), seed);
}

inline uint32_t
rgph_u32_murmur32s_f64(double value, uint32_t seed)
{

	return rgph_u32_murmur32s_u64(rgph_d2u64(value), seed);
}

inline uint32_t
rgph_u32_murmur32s_data(const void *data, size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	const uint8_t *end = key + len;
	uint32_t k, h[1] = { seed };
#if defined(UNALIGNED_READ)
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
#if !defined(UNALIGNED_READ)
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
rgph_u32_murmur32s_data32(const void *data, size_t len, uint32_t seed)
{
	const uint32_t *key = (const uint32_t *)(const char *)data;
	const uint32_t *end = key + len;
	uint32_t h[1] = { seed };

	for (; key != end; key += 1)
		rgph_murmur32s_mix(htole32(key[0]), h, 0);

	rgph_murmur32s_mix(0, h, 1);
	rgph_murmur32s_finalise(len * sizeof(key[0]), h);

	return h[0];
}

inline uint32_t
rgph_u32_murmur32s_data64(const void *data, size_t len, uint32_t seed)
{
	const uint64_t *key = (const uint64_t *)(const char *)data;
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

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint32_t rgph_u32_murmur32s_u8a(const uint8_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u32_murmur32s_data")));
#else
uint32_t
rgph_u32_murmur32s_u8a(const uint8_t *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data(key, len, seed);
}
#endif

inline uint32_t
rgph_u32_murmur32s_u16a(const uint16_t *key, size_t len, uint32_t seed)
{
	const uint8_t *arg = (const uint8_t *)(const char *)key;

	// "add %rsi,%rsi; jmp rgph_u32_murmur32s_u8a" is smaller than inlined
	//return rgph_u32_murmur32s_data(key, len * sizeof(key[0]), seed);
	return rgph_u32_murmur32s_u8a(arg, len * sizeof(key[0]), seed);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint32_t rgph_u32_murmur32s_u32a(const uint32_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u32_murmur32s_data32")));
#else
uint32_t
rgph_u32_murmur32s_u32a(const uint32_t *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint32_t rgph_u32_murmur32s_u64a(const uint64_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u32_murmur32s_data64")));
#else
uint32_t
rgph_u32_murmur32s_u64a(const uint64_t *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data64(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint32_t rgph_u32_murmur32s_f32a(const float *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u32_murmur32s_data32")));
#else
uint32_t
rgph_u32_murmur32s_f32a(const float *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint32_t rgph_u32_murmur32s_f64a(const double *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u32_murmur32s_data64")));
#else
uint32_t
rgph_u32_murmur32s_f64a(const double *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data64(key, len, seed);
}
#endif

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

	rgph_u8x4_murmur32s_u32(rgph_f2u32(value), seed, h8);
}

void
rgph_u8x4_murmur32s_f64(double value, uint32_t seed, uint8_t *h8)
{

	rgph_u8x4_murmur32s_u64(rgph_d2u64(value), seed, h8);
}

inline void
rgph_u8x4_murmur32s_data(const void * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

inline void
rgph_u8x4_murmur32s_data32(const void * restrict data,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data32(data, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

inline void
rgph_u8x4_murmur32s_data64(const void * restrict data,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data64(data, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u8x4_murmur32s_u8a(const uint8_t *, size_t, uint32_t,
    uint8_t *) __attribute__((weak,alias("rgph_u8x4_murmur32s_data")));
#else
void
rgph_u8x4_murmur32s_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{

	rgph_u8x4_murmur32s_data(key, len, seed, h8);
}
#endif

void
rgph_u8x4_murmur32s_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{
	const uint8_t *arg = (const uint8_t *)(const char *)key;

	// "add %rsi,%rsi; jmp rgph_u8x4_murmur32s_u8a" is smaller than inlined
	//rgph_u8x4_murmur32s_data(key, len * sizeof(key[0]), seed, h8);
	rgph_u8x4_murmur32s_u8a(arg, len * sizeof(key[0]), seed, h8);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u8x4_murmur32s_u32a(const uint32_t *, size_t, uint32_t,
    uint8_t *) __attribute__((weak,alias("rgph_u8x4_murmur32s_data32")));
#else
void
rgph_u8x4_murmur32s_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{

	rgph_u8x4_murmur32s_data32(key, len, seed, h8);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u8x4_murmur32s_u64a(const uint64_t *, size_t, uint32_t,
    uint8_t *) __attribute__((weak,alias("rgph_u8x4_murmur32s_data64")));
#else
void
rgph_u8x4_murmur32s_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{

	rgph_u8x4_murmur32s_data64(key, len, seed, h8);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u8x4_murmur32s_f32a(const float *, size_t, uint32_t,
    uint8_t *) __attribute__((weak,alias("rgph_u8x4_murmur32s_data32")));
#else
void
rgph_u8x4_murmur32s_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{

	rgph_u8x4_murmur32s_data32(key, len, seed, h8);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u8x4_murmur32s_f64a(const double *, size_t, uint32_t,
    uint8_t *) __attribute__((weak,alias("rgph_u8x4_murmur32s_data64")));
#else
void
rgph_u8x4_murmur32s_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint8_t * restrict h8)
{

	rgph_u8x4_murmur32s_data64(key, len, seed, h8);
}
#endif

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

	rgph_u16x2_murmur32s_u32(rgph_f2u32(value), seed, h16);
}

void
rgph_u16x2_murmur32s_f64(double value, uint32_t seed, uint16_t *h16)
{

	rgph_u16x2_murmur32s_u64(rgph_d2u64(value), seed, h16);
}

inline void
rgph_u16x2_murmur32s_data(const void * restrict data,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data(data, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

inline void
rgph_u16x2_murmur32s_data32(const void * restrict data,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data32(data, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

inline void
rgph_u16x2_murmur32s_data64(const void * restrict data,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data64(data, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x2_murmur32s_u8a(const uint8_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x2_murmur32s_data")));
#else
void
rgph_u16x2_murmur32s_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{

	rgph_u16x2_murmur32s_data(key, len, seed, h16);
}
#endif

void
rgph_u16x2_murmur32s_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{
	const uint8_t *arg = (const uint8_t *)(const char *)key;

	// "add %rsi,%rsi; jmp rgph_u16x2_murmur32s_u8a" is smaller than inlined
	//rgph_u16x2_murmur32s_data(key, len * sizeof(key[0]), seed, h16);
	rgph_u16x2_murmur32s_u8a(arg, len * sizeof(key[0]), seed, h16);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x2_murmur32s_u32a(const uint32_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x2_murmur32s_data32")));
#else
void
rgph_u16x2_murmur32s_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{

	rgph_u16x2_murmur32s_data32(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x2_murmur32s_u64a(const uint64_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x2_murmur32s_data64")));
#else
void
rgph_u16x2_murmur32s_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{

	rgph_u16x2_murmur32s_data64(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x2_murmur32s_f32a(const float *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x2_murmur32s_data32")));
#else
void
rgph_u16x2_murmur32s_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{

	rgph_u16x2_murmur32s_data32(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x2_murmur32s_f64a(const double *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x2_murmur32s_data64")));
#else
void
rgph_u16x2_murmur32s_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint16_t * restrict h16)
{

	rgph_u16x2_murmur32s_data64(key, len, seed, h16);
}
#endif
