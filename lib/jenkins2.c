/*-
 * Copyright (c) 2014-2015 Alexander Nasonov.
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
#include "hash_priv.h"
#include "rgph_hash.h"

#include <stddef.h>
#include <stdint.h>

#define SEED1(seed) RGPH_JENKINS2_SEED1
#define SEED2(seed) RGPH_JENKINS2_SEED2
#define SEED3(seed) seed

/* XXX Move to rhpg_hash.h. */
#define RGPH_JENKINS2_MIX(a, b, c) do { \
        a -= b; a -= c; a ^= (c >> 13); \
        b -= c; b -= a; b ^= (a << 8);  \
        c -= a; c -= b; c ^= (b >> 13); \
        a -= b; a -= c; a ^= (c >> 12); \
        b -= c; b -= a; b ^= (a << 16); \
        c -= a; c -= b; c ^= (b >> 5);  \
        a -= b; a -= c; a ^= (c >> 3);  \
        b -= c; b -= a; b ^= (a << 10); \
        c -= a; c -= b; c ^= (b >> 15); \
} while (/* CONSTCOND */0)

inline void
rgph_u32x3_jenkins2_u8(uint8_t value, uint32_t seed, uint32_t *h)
{

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	h[0] += value;
	h[2] += sizeof(value);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}

inline void
rgph_u32x3_jenkins2_u16(uint16_t value, uint32_t seed, uint32_t *h)
{

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	h[0] += value;
	h[2] += sizeof(value);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}

inline void
rgph_u32x3_jenkins2_u32(uint32_t value, uint32_t seed, uint32_t *h)
{

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	h[0] += value;
	h[2] += sizeof(value);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}

inline void
rgph_u32x3_jenkins2_u64(uint64_t value, uint32_t seed, uint32_t *h)
{

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	h[0] += value & UINT32_MAX;
	h[1] += value >> 32;
	h[2] += sizeof(value);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}

inline void
rgph_u32x3_jenkins2_f32(float value, uint32_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2_u32(f2u32(value), seed, h);
}

inline void
rgph_u32x3_jenkins2_f64(double value, uint32_t seed, uint32_t *h)
{

	rgph_u32x3_jenkins2_u64(d2u64(value), seed, h);
}

uint32_t
rgph_u32_jenkins2_u8(uint8_t value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_u8(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_jenkins2_u16(uint16_t value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_u16(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_jenkins2_u32(uint32_t value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_u32(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_jenkins2_u64(uint64_t value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_u64(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_jenkins2_f32(float value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_f32(value, seed, h);
	return h[0];
}

uint32_t
rgph_u32_jenkins2_f64(double value, uint32_t seed)
{
	uint32_t h[3];

	rgph_u32x3_jenkins2_f64(value, seed, h);
	return h[0];
}

uint64_t
rgph_u64_jenkins2_u8(uint8_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u8(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_jenkins2_u16(uint16_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u16(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_jenkins2_u32(uint32_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u32(value, seed, h);
	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_jenkins2_u64(uint64_t value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u64(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_jenkins2_f32(float value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_f32(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

uint64_t
rgph_u64_jenkins2_f64(double value, uint32_t seed)
{
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_f64(value, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

inline void
rgph_u32x3_jenkins2_u8a(const uint8_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint8_t *end = key + len;
	int n = 0;

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

#if !defined(RGPH_UNALIGNED_READ)
	if ((len & 3) == 0) {
		for (; end - key >= 12; key += 12) {
			h[0] += htole32(*((const uint32_t *)&key[0]));
			h[1] += htole32(*((const uint32_t *)&key[4]));
			h[2] += htole32(*((const uint32_t *)&key[8]));
			RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		}

		if (end - key >= 4) {
			h[n++] += htole32(*((const uint32_t *)&key[0]));
			key += 4;
		}
		if (end - key >= 4) {
			h[n++] += htole32(*((const uint32_t *)&key[0]));
			key += 4;
		}
	} else {
#endif
		for (; end - key >= 12; key += 12) {
			h[0] += read32(&key[0]);
			h[1] += read32(&key[4]);
			h[2] += read32(&key[8]);
			RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		}

		if (end - key >= 4) {
			h[n++] += read32(&key[0]);
			key += 4;
		}
		if (end - key >= 4) {
			h[n++] += read32(&key[0]);
			key += 4;
		}
#if !defined(RGPH_UNALIGNED_READ)
	}
#endif

	switch (end - key) {
	case 3:
		h[n] += (uint32_t)key[2] << 16; /* FALLTHROUGH */
	case 2:
		h[n] += (uint32_t)key[1] << 8;  /* FALLTHROUGH */
	case 1:
		h[n] += (uint32_t)key[0];       /* FALLTHROUGH */
	case 0:
		h[2] += len;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}
}

inline void
rgph_u32x3_jenkins2_data(const void * restrict data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;

	rgph_u32x3_jenkins2_u8a(key, len, seed, h);
}

uint32_t
rgph_u32_jenkins2_data(const void * restrict data,
    size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u8a(key, len, seed, h);
	return h[0];
}

uint64_t
rgph_u64_jenkins2_data(const void * restrict data,
    size_t len, uint32_t seed)
{
	const uint8_t *key = (const uint8_t *)(const char *)data;
	uint64_t res;
	uint32_t h[3];

	rgph_u32x3_jenkins2_u8a(key, len, seed, h);

	res = h[1];
	res = h[0] | (res << 32);
	return res;
}

void
rgph_u32x3_jenkins2_u16a(const uint16_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{

	return rgph_u32x3_jenkins2_data((const void *)key,
	    len * sizeof(key[0]), seed, h);
}

void
rgph_u32x3_jenkins2_u32a(const uint32_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint32_t *end = key + len;

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	for (; end - key >= 3; key += 3) {
		h[0] += htole32(key[0]);
		h[1] += htole32(key[1]);
		h[2] += htole32(key[2]);
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}

	switch (end - key) {
	case 2:
		h[1] += htole32(key[1]); /* FALLTHROUGH */
	case 1:
		h[0] += htole32(key[0]); /* FALLTHROUGH */
	case 0:
		h[2] += len * sizeof(key[0]);
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}
}

void
rgph_u32x3_jenkins2_u64a(const uint64_t * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint64_t *end = key + len;
	uint64_t a, b, c;

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	for (; end - key >= 3; key += 3) {
		a = htole64(key[0]);
		b = htole64(key[1]);
		c = htole64(key[2]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		h[0] += b >> 32;
		h[1] += c & UINT32_MAX;
		h[2] += c >> 32;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}

	switch (end - key) {
	case 2:
		a = htole64(key[0]);
		b = htole64(key[1]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		h[0] += b >> 32;
		break;
	case 1:
		a = htole64(key[0]);
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		break;
	}

	h[2] += len * sizeof(key[0]);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}

void
rgph_u32x3_jenkins2_f32a(const float * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const float *end = key + len;

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	for (; end - key >= 3; key += 3) {
		h[0] += htole32(f2u32(key[0]));
		h[1] += htole32(f2u32(key[1]));
		h[2] += htole32(f2u32(key[2]));
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}

	switch (end - key) {
	case 2:
		h[1] += htole32(f2u32(key[1])); /* FALLTHROUGH */
	case 1:
		h[0] += htole32(f2u32(key[0])); /* FALLTHROUGH */
	case 0:
		h[2] += len * sizeof(key[0]);
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}
}

void
rgph_u32x3_jenkins2_f64a(const double * restrict key,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const double *end = key + len;
	uint64_t a, b, c;

	h[0] = SEED1(seed);
	h[1] = SEED2(seed);
	h[2] = SEED3(seed);

	for (; end - key >= 3; key += 3) {
		a = htole64(d2u64(key[0]));
		b = htole64(d2u64(key[1]));
		c = htole64(d2u64(key[2]));
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		h[0] += b >> 32;
		h[1] += c & UINT32_MAX;
		h[2] += c >> 32;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
	}

	switch (end - key) {
	case 2:
		a = htole64(d2u64(key[0]));
		b = htole64(d2u64(key[1]));
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		h[2] += b & UINT32_MAX;
		RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
		h[0] += b >> 32;
		break;
	case 1:
		a = htole64(d2u64(key[0]));
		h[0] += a & UINT32_MAX;
		h[1] += a >> 32;
		break;
	}

	h[2] += len * sizeof(key[0]);
	RGPH_JENKINS2_MIX(h[0], h[1], h[2]);
}
