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

#include "rgph_hash_impl.h"
#include "rgph_hash.h"

#include <stddef.h>
#include <stdint.h>

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
/* Non-public external symbols for aliasing. */
void rgph_u32x4_murmur32_data32(const void *, size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32_data64(const void *, size_t, uint32_t, uint32_t *);
#endif

inline void
rgph_u32x4_murmur32_u8(uint8_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(value, h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u16(uint16_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(htole16(value), h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u32(uint32_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(htole32(value), h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_u64(uint64_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix1(htole64(value) >> 32, h);
	rgph_murmur32_mix0(htole64(value) & UINT32_MAX, h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32_f32(float value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_u32(rgph_f2u32(value), seed, h);
}

inline void
rgph_u32x4_murmur32_f64(double value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_u64(rgph_d2u64(value), seed, h);
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
rgph_u32x4_murmur32_data(const void *data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint8_t * restrict key = data;
	const uint8_t *end = key + len;
	uint32_t w[4];
#if defined(UNALIGNED_READ)
	const int down = 0;
#else
	const int down = ((uintptr_t)key) & 3;
	uint32_t carry = len > 0 && down != 0 ? rgph_read32a(key - down) : 0;
#endif

	h[0] = h[1] = h[2] = h[3] = seed;

	if (down == 0) {
		for (; end - key >= 16; key += 16) {
			w[0] = rgph_read32a(&key[0]);
			w[1] = rgph_read32a(&key[4]);
			w[2] = rgph_read32a(&key[8]);
			w[3] = rgph_read32a(&key[12]);
			rgph_murmur32_mix(w, h);
		}
	} else {
#if !defined(UNALIGNED_READ)
		for (; end - key >= 16; key += 16) {
			rgph_read32u(key, 4 - down, &carry, w, 4);
			rgph_murmur32_mix(w, h);
		}
#endif
	}

	w[0] = w[1] = w[2] = w[3] = 0;

	switch (end - key) {
	case 15: w[3] ^= key[14] << 16;
	case 14: w[3] ^= key[13] << 8;
	case 13: w[3] ^= key[12];
	         rgph_murmur32_mix3(w[3], h);
	         /* FALLTHROUGH */
	case 12: w[2] ^= key[11] << 24;
	case 11: w[2] ^= key[10] << 16;
	case 10: w[2] ^= key[9] << 8;
	case  9: w[2] ^= key[8];
	         rgph_murmur32_mix2(w[2], h);
	         /* FALLTHROUGH */
	case  8: w[1] ^= key[7] << 24;
	case  7: w[1] ^= key[6] << 16;
	case  6: w[1] ^= key[5] << 8;
	case  5: w[1] ^= key[4];
	      rgph_murmur32_mix1(w[1], h);
	      /* FALLTHROUGH */
	case  4: w[0] ^= key[3] << 24;
	case  3: w[0] ^= key[2] << 16;
	case  2: w[0] ^= key[1] << 8;
	case  1: w[0] ^= key[0];
	         rgph_murmur32_mix0(w[0], h);
	         /* FALLTHROUGH */
	case 0:
	         rgph_murmur32_finalise(len, h);
	}
}

inline void
rgph_u32x4_murmur32_data32(const void *data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint32_t * restrict key = data;
	const uint32_t *end = key + len;
	uint32_t w[4];

	h[0] = h[1] = h[2] = h[3] = seed;

	for (; end - key >= 4; key += 4) {
		w[0] = htole32(key[0]);
		w[1] = htole32(key[1]);
		w[2] = htole32(key[2]);
		w[3] = htole32(key[3]);
		rgph_murmur32_mix(w, h);
	}

	switch (end - key) {
	case 3: w[2] = htole32(key[2]);
	        rgph_murmur32_mix2(w[2], h);
	        /* FALLTHROUGH */
	case 2: w[1] = htole32(key[1]);
	        rgph_murmur32_mix1(w[1], h);
	        /* FALLTHROUGH */
	case 1: w[0] = htole32(key[0]);
	        rgph_murmur32_mix0(w[0], h);
	        /* FALLTHROUGH */
	case 0:
		rgph_murmur32_finalise(len * sizeof(key[0]), h);
	}
}

inline void
rgph_u32x4_murmur32_data64(const void *data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint64_t * restrict key = data;
	const uint64_t *end = key + len;
	uint32_t w[4];
	uint64_t a, b;

	h[0] = h[1] = h[2] = h[3] = seed;

	for (; end - key >= 2; key += 2) {
		a = htole64(key[0]);
		b = htole64(key[1]);
		w[0] = a & UINT32_MAX;
		w[1] = a >> 32;
		w[2] = b & UINT32_MAX;
		w[3] = b >> 32;
		rgph_murmur32_mix(w, h);
	}

	switch (end - key) {
	case 1: a = htole64(key[0]);
	        w[0] = a & UINT32_MAX;
	        w[1] = a >> 32;
	        rgph_murmur32_mix1(w[1], h);
	        rgph_murmur32_mix0(w[0], h);
	        /* FALLTHROUGH */
	case 0:
	        rgph_murmur32_finalise(len * sizeof(key[0]), h);
	}
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32_u8a(const uint8_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32_data")));
#else
void
rgph_u32x4_murmur32_u8a(const uint8_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_data(key, len, seed, h);
}
#endif

uint32_t
rgph_u32_murmur32_data(const void *data, size_t len, uint32_t seed)
{
	uint32_t h[4];

	rgph_u32x4_murmur32_data(data, len, seed, h);
	return h[0];
}

uint64_t
rgph_u64_murmur32_data(const void *data, size_t len, uint32_t seed)
{
	uint64_t res;
	uint32_t h[4];

	rgph_u32x4_murmur32_data(data, len, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

void
rgph_u32x4_murmur32_u16a(const uint16_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	// "add %rsi,%rsi; jmp rgph_u32x4_murmur32_u8a" is smaller than inlined
	//rgph_u32x4_murmur32_data(key, len * sizeof(key[0]), seed, h);
	rgph_u32x4_murmur32_u8a(arg, len * sizeof(key[0]), seed, h);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32_u32a(const uint32_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32_data32")));
#else
void
rgph_u32x4_murmur32_u32a(const uint32_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32_u64a(const uint64_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32_data64")));
#else
void
rgph_u32x4_murmur32_u64a(const uint64_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32_data64(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32_f32a(const float *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32_data32")));
#else
void
rgph_u32x4_murmur32_f32a(const float *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	return rgph_u32x4_murmur32_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32_f64a(const double *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32_data64")));
#else
void
rgph_u32x4_murmur32_f64a(const double *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	return rgph_u32x4_murmur32_data64(key, len, seed, h);
}
#endif
