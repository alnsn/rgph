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
void rgph_u32x4_murmur32v_data32(const void *, size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_data64(const void *, size_t, uint32_t, uint32_t *);
#endif


void
rgph_u32x4_murmur32v_u8(uint8_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(value, h);
	rgph_murmur32_finalise(sizeof(value), h);
}

void
rgph_u32x4_murmur32v_u16(uint16_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(htole16(value), h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32v_u32(uint32_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix0(htole32(value), h);
	rgph_murmur32_finalise(sizeof(value), h);
}

inline void
rgph_u32x4_murmur32v_u64(uint64_t value, uint32_t seed, uint32_t *h)
{

	h[0] = h[1] = h[2] = h[3] = seed;

	rgph_murmur32_mix1(htole64(value) >> 32, h);
	rgph_murmur32_mix0(htole64(value) & UINT32_MAX, h);
	rgph_murmur32_finalise(sizeof(value), h);
}

void
rgph_u32x4_murmur32v_f32(float value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32v_u32(rgph_f2u32(value), seed, h);
}

void
rgph_u32x4_murmur32v_f64(double value, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32v_u64(rgph_d2u64(value), seed, h);
}

inline void
rgph_u32x4_murmur32v_data32(const void *data,
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
rgph_u32x4_murmur32v_data64(const void *data,
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

void
rgph_u32x4_murmur32v_u8a(const uint8_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32v_data(key, len, seed, h);
}

void
rgph_u32x4_murmur32v_u16a(const uint16_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	rgph_u32x4_murmur32v_data(arg, len * sizeof(key[0]), seed, h);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32v_u32a(const uint32_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32v_data32")));
#else
void
rgph_u32x4_murmur32v_u32a(const uint32_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32v_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32v_u64a(const uint64_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32v_data64")));
#else
void
rgph_u32x4_murmur32v_u64a(const uint64_t *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	rgph_u32x4_murmur32v_data64(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32v_f32a(const float *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32v_data32")));
#else
void
rgph_u32x4_murmur32v_f32a(const float *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	return rgph_u32x4_murmur32v_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x4_murmur32v_f64a(const double *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x4_murmur32v_data64")));
#else
void
rgph_u32x4_murmur32v_f64a(const double *key,
    size_t len, uint32_t seed, uint32_t *h)
{

	return rgph_u32x4_murmur32v_data64(key, len, seed, h);
}
#endif
