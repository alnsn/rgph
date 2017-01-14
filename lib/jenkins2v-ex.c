/*-
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 *
 * Copyright (c) 2014-2016 Alexander Nasonov.
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
 * Based on http://www.burtleburtle.net/bob/c/lookup2.c.
 */
#include "rgph_hash_impl.h"
#include "rgph_hash.h"

#include <stddef.h>
#include <stdint.h>


#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
/* Non-public external symbols for aliasing. */
void rgph_u32x3_jenkins2v_data32(const void *, size_t, uintptr_t, uint32_t *);
void rgph_u32x3_jenkins2v_data64(const void *, size_t, uintptr_t, uint32_t *);
#endif


void
rgph_u32x3_jenkins2v_u8(uint8_t value, uintptr_t seed, uint32_t *h)
{

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	h[0] += value;
	h[2] += sizeof(value);
	rgph_jenkins2_mix(h);
}

void
rgph_u32x3_jenkins2v_u16(uint16_t value, uintptr_t seed, uint32_t *h)
{

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	h[0] += htole16(value);
	h[2] += sizeof(value);
	rgph_jenkins2_mix(h);
}

inline void
rgph_u32x3_jenkins2v_u32(uint32_t value, uintptr_t seed, uint32_t *h)
{

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	h[0] += htole32(value);
	h[2] += sizeof(value);
	rgph_jenkins2_mix(h);
}

inline void
rgph_u32x3_jenkins2v_u64(uint64_t value, uintptr_t seed, uint32_t *h)
{

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	h[0] += htole64(value) & UINT32_MAX;
	h[1] += htole64(value) >> 32;
	h[2] += sizeof(value);
	rgph_jenkins2_mix(h);
}

void
rgph_u32x3_jenkins2v_f32(float value, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_u32(rgph_f2u32(value), seed, h);
}

void
rgph_u32x3_jenkins2v_f64(double value, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_u64(rgph_d2u64(value), seed, h);
}

inline void
rgph_u32x3_jenkins2v_data32(const void *data,
    size_t len, uintptr_t seed, uint32_t * restrict h)
{
	const uint32_t * restrict key = data;
	const uint32_t *end = key + len;

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	for (; end - key >= 3; key += 3) {
		h[0] += htole32(key[0]);
		h[1] += htole32(key[1]);
		h[2] += htole32(key[2]);
		rgph_jenkins2_mix(h);
	}

	switch (end - key) {
	case 2:
		h[1] += htole32(key[1]); /* FALLTHROUGH */
	case 1:
		h[0] += htole32(key[0]); /* FALLTHROUGH */
	case 0:
		h[2] += len * sizeof(key[0]);
		rgph_jenkins2_mix(h);
	}
}

inline void
rgph_u32x3_jenkins2v_data64(const void *data,
    size_t len, uintptr_t seed, uint32_t * restrict h)
{
	const uint64_t * restrict key = data;
	const uint64_t *end = key + len;
	uint64_t a, b, c;

	h[0] = RGPH_JENKINS2V_SEED1;
	h[1] = RGPH_JENKINS2V_SEED2;
	h[2] = seed;

	for (; end - key >= 3; key += 3) {
		a = htole64(key[0]);
		b = htole64(key[1]);
		c = htole64(key[2]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		rgph_jenkins2_mix(h);
		h[0] += b >> 32;
		h[1] += c & UINT32_MAX;
		h[2] += c >> 32;
		rgph_jenkins2_mix(h);
	}

	switch (end - key) {
	case 2:
		a = htole64(key[0]);
		b = htole64(key[1]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		rgph_jenkins2_mix(h);
		h[0] += b >> 32;
		break;
	case 1:
		a = htole64(key[0]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		break;
	}

	h[2] += len * sizeof(key[0]);
	rgph_jenkins2_mix(h);
}


void
rgph_u32x3_jenkins2v_u8a(const uint8_t *key,
    size_t len, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_data(key, len, seed, h);
}

void
rgph_u32x3_jenkins2v_u16a(const uint16_t *key,
    size_t len, uintptr_t seed, uint32_t *h)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	rgph_u32x3_jenkins2v_data(arg, len * sizeof(key[0]), seed, h);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x3_jenkins2v_u32a(const uint32_t *, size_t, uintptr_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x3_jenkins2v_data32")));
#else
void
rgph_u32x3_jenkins2v_u32a(const uint32_t *key,
    size_t len, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x3_jenkins2v_u64a(const uint64_t *, size_t, uintptr_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x3_jenkins2v_data64")));
#else
void
rgph_u32x3_jenkins2v_u64a(const uint64_t *key,
    size_t len, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_data64(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x3_jenkins2v_f32a(const float *, size_t, uintptr_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x3_jenkins2v_data32")));
#else
void
rgph_u32x3_jenkins2v_f32a(const float *key,
    size_t len, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_data32(key, len, seed, h);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x3_jenkins2v_f64a(const double *, size_t, uintptr_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x3_jenkins2v_data64")));
#else
void
rgph_u32x3_jenkins2v_f64a(const double *key,
    size_t len, uintptr_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2v_data64(key, len, seed, h);
}
#endif
