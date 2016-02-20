/*-
 * xxHash - Fast Hash algorithm
 * Copyright (C) 2012-2014, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 * Copyright (c) 2016 Alexander Nasonov.
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
uint64_t rgph_u64_xxh64s_data32(const void *, size_t, uint32_t);
uint64_t rgph_u64_xxh64s_data64(const void *, size_t, uint32_t);
void rgph_u16x4_xxh64s_data32(const void *, size_t, uint32_t, uint16_t *);
void rgph_u16x4_xxh64s_data64(const void *, size_t, uint32_t, uint16_t *);
void rgph_u32x2_xxh64s_data32(const void *, size_t, uint32_t, uint32_t *);
void rgph_u32x2_xxh64s_data64(const void *, size_t, uint32_t, uint32_t *);
#endif


inline uint64_t
rgph_u64_xxh64s_data(const void *data, size_t len, uint32_t seed)
{
	const uint8_t *key = data;
	const uint8_t *end = key + len;
	uint64_t w[4], h[4];
#if defined(UNALIGNED_READ)
	const int down = 0;
#else
	const int down = ((uintptr_t)key) & 7;
	uint64_t carry = len > 0 && down != 0 ? rgph_read64a(key - down) : 0;
#endif

	rgph_xxh64s_init(len, seed, h);

	if (down == 0) {
		for (; end - key >= 32; key += 32) {
			w[0] = rgph_read64a(&key[0]);
			w[1] = rgph_read64a(&key[8]);
			w[2] = rgph_read64a(&key[16]);
			w[3] = rgph_read64a(&key[24]);
			rgph_xxh64s_mix(w, h);
		}
	} else {
#if !defined(UNALIGNED_READ)
		for (; end - key >= 32; key += 32) {
			rgph_read64u(key, 8 - down, &carry, w, 4);
			rgph_xxh64s_mix(w, h);
		}
#endif
	}

	rgph_xxh64s_fold(len, h);

	if (down == 0) {
		for (; end - key >= 8; key += 8) {
			w[0] = rgph_read64a(&key[0]);
			rgph_xxh64s_fmix8(w[0], h);
		}
	} else {
#if !defined(UNALIGNED_READ)
		for (; end - key >= 8; key += 8) {
			rgph_read64u(key, 8 - down, &carry, w, 1);
			rgph_xxh64s_fmix8(w[0], h);
		}
#endif
	}

	if (end - key >= 4) {
		/* Switch to 32bit shadows. */
		uint32_t w[1];
#if defined(UNALIGNED_READ)

		w[0] = rgph_read32a(&key[0]);
#else
		const int down = ((uintptr_t)key) & 3;
		uint32_t carry = down != 0 ? rgph_read32a(key - down) : 0;

		if (down == 0)
			w[0] = rgph_read32a(&key[0]);
		else
			rgph_read32u(key, 4 - down, &carry, w, 1);
#endif
		rgph_xxh64s_fmix4(w[0], h);
		key += 4;
	}

	for (; end - key >= 1; key += 1)
		rgph_xxh64s_fmix1(key[0], h);

	rgph_xxh64s_finalise(h);

	return h[0];
}

inline void
rgph_u16x4_xxh64s_data(const void *key,
    size_t len, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data(key, len, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

inline void
rgph_u32x2_xxh64s_data(const void *data,
    size_t len, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data(data, len, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

inline uint64_t
rgph_u64_xxh64s_u8(uint8_t value, uint32_t seed)
{
	uint64_t h[4];

	rgph_xxh64s_init(sizeof(value), seed, h);
	rgph_xxh64s_fold(sizeof(value), h);

	rgph_xxh64s_fmix1(value, h);
	rgph_xxh64s_finalise(h);

	return h[0];
}

inline uint64_t
rgph_u64_xxh64s_u16(uint16_t value, uint32_t seed)
{
	uint64_t h[4];

	rgph_xxh64s_init(sizeof(value), seed, h);
	rgph_xxh64s_fold(sizeof(value), h);

	rgph_xxh64s_fmix1(htole16(value) & 0xff, h);
	rgph_xxh64s_fmix1(htole16(value) >> 8, h);
	rgph_xxh64s_finalise(h);

	return h[0];
}

inline uint64_t
rgph_u64_xxh64s_u32(uint32_t value, uint32_t seed)
{
	uint64_t h[4];

	rgph_xxh64s_init(sizeof(value), seed, h);
	rgph_xxh64s_fold(sizeof(value), h);

	rgph_xxh64s_fmix4(htole32(value), h);
	rgph_xxh64s_finalise(h);

	return h[0];
}

inline uint64_t
rgph_u64_xxh64s_u64(uint64_t value, uint32_t seed)
{
	uint64_t h[4];

	rgph_xxh64s_init(sizeof(value), seed, h);
	rgph_xxh64s_fold(sizeof(value), h);

	rgph_xxh64s_fmix8(value, h);
	rgph_xxh64s_finalise(h);

	return h[0];
}

inline uint64_t
rgph_u64_xxh64s_f32(float value, uint32_t seed)
{

	return rgph_u64_xxh64s_u32(rgph_f2u32(value), seed);
}

inline uint64_t
rgph_u64_xxh64s_f64(double value, uint32_t seed)
{

	return rgph_u64_xxh64s_u64(rgph_d2u64(value), seed);
}

inline uint64_t
rgph_u64_xxh64s_data32(const void *data, size_t len, uint32_t seed)
{
	const uint32_t *key = data;
	const uint32_t *end = key + len;
	uint64_t w[4], h[4];

	rgph_xxh64s_init(len * sizeof(key[0]), seed, h);

	for (; end - key >= 8; key += 8) {
		w[0] = htole32(key[0]) + ((uint64_t)htole32(key[1]) << 32);
		w[1] = htole32(key[2]) + ((uint64_t)htole32(key[3]) << 32);
		w[2] = htole32(key[4]) + ((uint64_t)htole32(key[5]) << 32);
		w[3] = htole32(key[6]) + ((uint64_t)htole32(key[7]) << 32);
		rgph_xxh64s_mix(w, h);
	}

	rgph_xxh64s_fold(len * sizeof(key[0]), h);

	for (; end - key >= 2; key += 2) {
		w[0] = htole32(key[0]) + ((uint64_t)htole32(key[1]) << 32);
		rgph_xxh64s_fmix8(w[0], h);
	}

	if (end - key >= 1)
		rgph_xxh64s_fmix4(htole32(key[0]), h);

	rgph_xxh64s_finalise(h);
	return h[0];
}

inline uint64_t
rgph_u64_xxh64s_data64(const void *data, size_t len, uint32_t seed)
{
	const uint64_t *key = data;
	const uint64_t *end = key + len;
	uint64_t w[4], h[4];

	rgph_xxh64s_init(len * sizeof(key[0]), seed, h);

	for (; end - key >= 4; key += 4) {
		w[0] = htole64(key[0]);
		w[1] = htole64(key[1]);
		w[2] = htole64(key[2]);
		w[3] = htole64(key[3]);
		rgph_xxh64s_mix(w, h);
	}

	rgph_xxh64s_fold(len * sizeof(key[0]), h);

	for (; end - key >= 1; key += 1)
		rgph_xxh64s_fmix8(htole64(key[0]), h);

	rgph_xxh64s_finalise(h);
	return h[0];
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_xxh64s_u8a(const uint8_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u64_xxh64s_data")));
#else
uint64_t
rgph_u64_xxh64s_u8a(const uint8_t *key, size_t len, uint32_t seed)
{

	return rgph_u64_xxh64s_data(key, len, seed);
}
#endif

inline uint64_t
rgph_u64_xxh64s_u16a(const uint16_t *key, size_t len, uint32_t seed)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	// "add %rsi,%rsi; jmp rgph_u64_xxh64s_u8a" is smaller than inlined
	//return rgph_u64_xxh64s_data(key, len * sizeof(key[0]), seed);
	return rgph_u64_xxh64s_u8a(arg, len * sizeof(key[0]), seed);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_xxh64s_u32a(const uint32_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u64_xxh64s_data32")));
#else
uint64_t
rgph_u64_xxh64s_u32a(const uint32_t *key, size_t len, uint32_t seed)
{

	return rgph_u64_xxh64s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_xxh64s_u64a(const uint64_t *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u64_xxh64s_data64")));
#else
uint64_t
rgph_u64_xxh64s_u64a(const uint64_t *key, size_t len, uint32_t seed)
{

	return rgph_u64_xxh64s_data64(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u32_xxh64s_f32a(const float *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u64_xxh64s_data32")));
#else
uint64_t
rgph_u64_xxh64s_f32a(const float *key, size_t len, uint32_t seed)
{

	return rgph_u64_xxh64s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_xxh64s_f64a(const double *, size_t,
    uint32_t) __attribute__((weak,alias("rgph_u64_xxh64s_data64")));
#else
uint64_t
rgph_u64_xxh64s_f64a(const double *key, size_t len, uint32_t seed)
{

	return rgph_u64_xxh64s_data64(key, len, seed);
}
#endif

void
rgph_u16x4_xxh64s_u8(uint8_t value, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u8(value, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

void
rgph_u16x4_xxh64s_u16(uint16_t value, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u16(value, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

inline void
rgph_u16x4_xxh64s_u32(uint32_t value, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u32(value, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

inline void
rgph_u16x4_xxh64s_u64(uint64_t value, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u64(value, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

void
rgph_u16x4_xxh64s_f32(float value, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_u32(rgph_f2u32(value), seed, h16);
}

void
rgph_u16x4_xxh64s_f64(double value, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_u64(rgph_d2u64(value), seed, h16);
}

inline void
rgph_u16x4_xxh64s_data32(const void *data,
    size_t len, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data32(data, len, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

inline void
rgph_u16x4_xxh64s_data64(const void *data,
    size_t len, uint32_t seed, uint16_t *h16)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data64(data, len, seed);
	h16[0] = (uint16_t)(h >> 48);
	h16[1] = (uint16_t)(h >> 32);
	h16[2] = (uint16_t)(h >> 16);
	h16[3] = (uint16_t)h;
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x4_xxh64s_u8a(const uint8_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x4_xxh64s_data")));
#else
void
rgph_u16x4_xxh64s_u8a(const uint8_t *key,
    size_t len, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_data(key, len, seed, h16);
}
#endif

void
rgph_u16x4_xxh64s_u16a(const uint16_t *key,
    size_t len, uint32_t seed, uint16_t *h16)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	// "add %rsi,%rsi; jmp rgph_u16x4_xxh64s_u8a" is smaller than inlined
	//rgph_u16x4_xxh64s_data(key, len * sizeof(key[0]), seed, h16);
	rgph_u16x4_xxh64s_u8a(arg, len * sizeof(key[0]), seed, h16);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x4_xxh64s_u32a(const uint32_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x4_xxh64s_data32")));
#else
void
rgph_u16x4_xxh64s_u32a(const uint32_t *key,
    size_t len, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_data32(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x4_xxh64s_u64a(const uint64_t *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x4_xxh64s_data64")));
#else
void
rgph_u16x4_xxh64s_u64a(const uint64_t *key,
    size_t len, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_data64(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x4_xxh64s_f32a(const float *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x4_xxh64s_data32")));
#else
void
rgph_u16x4_xxh64s_f32a(const float *key,
    size_t len, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_data32(key, len, seed, h16);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u16x4_xxh64s_f64a(const double *, size_t, uint32_t,
    uint16_t *) __attribute__((weak,alias("rgph_u16x4_xxh64s_data64")));
#else
void
rgph_u16x4_xxh64s_f64a(const double *key,
    size_t len, uint32_t seed, uint16_t *h16)
{

	rgph_u16x4_xxh64s_data64(key, len, seed, h16);
}
#endif

void
rgph_u32x2_xxh64s_u8(uint8_t value, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u8(value, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

void
rgph_u32x2_xxh64s_u16(uint16_t value, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u16(value, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

inline void
rgph_u32x2_xxh64s_u32(uint32_t value, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u32(value, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

inline void
rgph_u32x2_xxh64s_u64(uint64_t value, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_u64(value, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

void
rgph_u32x2_xxh64s_f32(float value, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_u32(rgph_f2u32(value), seed, h32);
}

void
rgph_u32x2_xxh64s_f64(double value, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_u64(rgph_d2u64(value), seed, h32);
}

inline void
rgph_u32x2_xxh64s_data32(const void *data,
    size_t len, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data32(data, len, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

inline void
rgph_u32x2_xxh64s_data64(const void *data,
    size_t len, uint32_t seed, uint32_t *h32)
{
	uint64_t h;

	h = rgph_u64_xxh64s_data64(data, len, seed);
	h32[0] = (uint32_t)(h >> 32);
	h32[1] = (uint32_t)h;
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x2_xxh64s_u8a(const uint8_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x2_xxh64s_data")));
#else
void
rgph_u32x2_xxh64s_u8a(const uint8_t *key,
    size_t len, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_data(key, len, seed, h32);
}
#endif

void
rgph_u32x2_xxh64s_u16a(const uint16_t *key,
    size_t len, uint32_t seed, uint32_t *h32)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	// "add %rsi,%rsi; jmp rgph_u32x2_xxh64s_u8a" is smaller than inlined
	//rgph_u32x2_xxh64s_data(key, len * sizeof(key[0]), seed, h32);
	rgph_u32x2_xxh64s_u8a(arg, len * sizeof(key[0]), seed, h32);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x2_xxh64s_u32a(const uint32_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x2_xxh64s_data32")));
#else
void
rgph_u32x2_xxh64s_u32a(const uint32_t *key,
    size_t len, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_data32(key, len, seed, h32);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x2_xxh64s_u64a(const uint64_t *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x2_xxh64s_data64")));
#else
void
rgph_u32x2_xxh64s_u64a(const uint64_t *key,
    size_t len, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_data64(key, len, seed, h32);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x2_xxh64s_f32a(const float *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x2_xxh64s_data32")));
#else
void
rgph_u16x2_xxh64s_f32a(const float *key,
    size_t len, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_data32(key, len, seed, h32);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
void rgph_u32x2_xxh64s_f64a(const double *, size_t, uint32_t,
    uint32_t *) __attribute__((weak,alias("rgph_u32x2_xxh64s_data64")));
#else
void
rgph_u32x2_xxh64s_f64a(const double *key,
    size_t len, uint32_t seed, uint32_t *h32)
{

	rgph_u32x2_xxh64s_data64(key, len, seed, h32);
}
#endif
