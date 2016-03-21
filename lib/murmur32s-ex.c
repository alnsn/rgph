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
uint32_t rgph_u32_murmur32s_data32(const void *, size_t, uint32_t);
uint32_t rgph_u32_murmur32s_data64(const void *, size_t, uint32_t);
#endif


uint32_t
rgph_u32_murmur32s_u8(uint8_t value, uint32_t seed)
{
	uint32_t h[1] = { seed };

	rgph_murmur32s_mix(value, h, 1);
	rgph_murmur32s_finalise(sizeof(value), h);

	return h[0];
}

uint32_t
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

uint32_t
rgph_u32_murmur32s_f32(float value, uint32_t seed)
{

	return rgph_u32_murmur32s_u32(rgph_f2u32(value), seed);
}

uint32_t
rgph_u32_murmur32s_f64(double value, uint32_t seed)
{

	return rgph_u32_murmur32s_u64(rgph_d2u64(value), seed);
}

inline uint32_t
rgph_u32_murmur32s_data32(const void *data, size_t len, uint32_t seed)
{
	const uint32_t *key = data;
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
	const uint64_t *key = data;
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

uint32_t
rgph_u32_murmur32s_u8a(const uint8_t *key, size_t len, uint32_t seed)
{

	return rgph_u32_murmur32s_data(key, len, seed);
}

uint32_t
rgph_u32_murmur32s_u16a(const uint16_t *key, size_t len, uint32_t seed)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	return rgph_u32_murmur32s_data(arg, len * sizeof(key[0]), seed);
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
