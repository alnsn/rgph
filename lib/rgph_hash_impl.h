/*-
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 *
 * xxHash - Fast Hash algorithm
 * Copyright (C) 2012-2014, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
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
#ifndef FILE_RGPH_HASH_IMPL_H_INCLUDED
#define FILE_RGPH_HASH_IMPL_H_INCLUDED

/* XXX https://gist.github.com/panzi/6856583 */
#if defined(__linux__)
#define _BSD_SOURCE
#include <endian.h>
#elif defined(__APPLE__)
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>
#define htole64(x) OSSwapHostToLittleInt64(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#else
#include <sys/endian.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <string.h> /* memcpy */

#define RGPH_JENKINS2_SEED1 UINT32_C(0x9e3779b9)
#define RGPH_JENKINS2_SEED2 UINT32_C(0x9e3779b9)

#define RGPH_MURMUR32_MUL1 UINT32_C(0x239b961b)
#define RGPH_MURMUR32_MUL2 UINT32_C(0xab0e9789)
#define RGPH_MURMUR32_MUL3 UINT32_C(0x38b34ae5)
#define RGPH_MURMUR32_MUL4 UINT32_C(0xa1e38b93)
#define RGPH_MURMUR32_ADD1 UINT32_C(0x561ccd1b)
#define RGPH_MURMUR32_ADD2 UINT32_C(0x0bcaa747)
#define RGPH_MURMUR32_ADD3 UINT32_C(0x96cd1c35)
#define RGPH_MURMUR32_ADD4 UINT32_C(0x32ac3b17)
#define RGPH_MURMUR32_FMIXMUL1 UINT32_C(0x85ebca6b)
#define RGPH_MURMUR32_FMIXMUL2 UINT32_C(0xc2b2ae35)

#define RGPH_MURMUR32S_MUL1 UINT32_C(0xcc9e2d51)
#define RGPH_MURMUR32S_MUL2 UINT32_C(0x1b873593)
#define RGPH_MURMUR32S_ADD1 UINT32_C(0xe6546b64)
#define RGPH_MURMUR32S_FMIXMUL1 UINT32_C(0x85ebca6b)
#define RGPH_MURMUR32S_FMIXMUL2 UINT32_C(0xc2b2ae35)

#define RGPH_XXH32S_PRIME1 UINT32_C(2654435761)
#define RGPH_XXH32S_PRIME2 UINT32_C(2246822519)
#define RGPH_XXH32S_PRIME3 UINT32_C(3266489917)
#define RGPH_XXH32S_PRIME4 UINT32_C(668265263)
#define RGPH_XXH32S_PRIME5 UINT32_C(374761393)

/* XXX Check with the C99 standard. */
#define rgph_unalias(T, p) ((T)(const char *)(p))

/*
 * Rotate left and right.
 */
#define rgph_rotl(x, l) (((x) << (l)) | ((x) >> (CHAR_BIT * sizeof(x) - (l))))
#define rgph_rotr(x, r) (((x) >> (r)) | ((x) << (CHAR_BIT * sizeof(x) - (r))))

/*
 * Read 32bit word from aligned pointer in little-endian order.
 */
#define rgph_read32a(aligned) \
	htole32(*(rgph_unalias(const uint32_t *, (aligned))))

/*
 * Read n 32bit unaligned words from ptr with a shift to align loads.
 * When reading a stream of words, the carry argument is normally
 * passed from the previous rgph_read32u() call. The value for the
 * first rgph_read32u() call must be loaded from the preceding aligned
 * word: curry = rgph_read32a(ptr + align_up - 4).
 * The behaviour is undefined when ptr is aligned.
 */
static inline void
rgph_read32u(const uint8_t * restrict ptr, int align_up,
    uint32_t *carry, uint32_t * restrict out, size_t n)
{
	const uint8_t *aligned = ptr + align_up;
	const int up_bits = 8 * align_up;
	const int down_bits = 32 - up_bits;
	size_t i;
	uint32_t w;

	for (i = 0; i < n; i++) {
		w = rgph_read32a(aligned);
		out[i] = (w << up_bits) | (*carry >> down_bits);
		aligned += 4;
		*carry = w;
	}
}

static inline uint32_t
rgph_f2u32(float f)
{
	uint32_t res;

#if !defined(__STRICT_ANSI__) /* XXX || C99+TC3 || C11 */
	union {
		float f;
		uint32_t u;
	} u = { f };

	res = u.u;
#else
	memcpy(&res, &f, sizeof(f));
#endif
	return res;
}

static inline uint64_t
rgph_d2u64(double d)
{
	uint64_t res;

#if !defined(__STRICT_ANSI__) /* XXX || C99+TC3 || C11 */
	/* Weird half little-endian half big-endian FP isn't supported. */
	union {
		double d;
		uint64_t u;
	} u = { d };

	res = u.u;
#else
	memcpy(&res, &d, sizeof(d));
#endif
	return res;
}

static inline void
rgph_jenkins2_mix(uint32_t h[/* static 3 */])
{

	h[0] -= h[1]; h[0] -= h[2]; h[0] ^= (h[2] >> 13);
	h[1] -= h[2]; h[1] -= h[0]; h[1] ^= (h[0] << 8);
	h[2] -= h[0]; h[2] -= h[1]; h[2] ^= (h[1] >> 13);
	h[0] -= h[1]; h[0] -= h[2]; h[0] ^= (h[2] >> 12);
	h[1] -= h[2]; h[1] -= h[0]; h[1] ^= (h[0] << 16);
	h[2] -= h[0]; h[2] -= h[1]; h[2] ^= (h[1] >> 5);
	h[0] -= h[1]; h[0] -= h[2]; h[0] ^= (h[2] >> 3);
	h[1] -= h[2]; h[1] -= h[0]; h[1] ^= (h[0] << 10);
	h[2] -= h[0]; h[2] -= h[1]; h[2] ^= (h[1] >> 15);
}

static inline uint32_t
rgph_murmur32_fmix(uint32_t h)
{

	h ^= h >> 16;
	h *= RGPH_MURMUR32_FMIXMUL1;
	h ^= h >> 13;
	h *= RGPH_MURMUR32_FMIXMUL2;
	h ^= h >> 16;

	return h;
}

static inline void
rgph_murmur32_finalise(size_t len, uint32_t h[/* static 4 */])
{

	/* Note that len > UINT32_MAX is truncated. */
	h[0] ^= len; h[1] ^= len; h[2] ^= len; h[3] ^= len;

	h[0] += h[1]; h[0] += h[2]; h[0] += h[3];
	h[1] += h[0]; h[2] += h[0]; h[3] += h[0];

	h[0] = rgph_murmur32_fmix(h[0]);
	h[1] = rgph_murmur32_fmix(h[1]);
	h[2] = rgph_murmur32_fmix(h[2]);
	h[3] = rgph_murmur32_fmix(h[3]);

	h[0] += h[1]; h[0] += h[2]; h[0] += h[3];
	h[1] += h[0]; h[2] += h[0]; h[3] += h[0];
}

static inline void
rgph_murmur32_mix0(uint32_t k, uint32_t h[/* static 4 */])
{

	k *= RGPH_MURMUR32_MUL1;
	k = rgph_rotl(k, 15);
	k *= RGPH_MURMUR32_MUL2;
	h[0] ^= k;
}

static inline void
rgph_murmur32_mix1(uint32_t k, uint32_t h[/* static 4 */])
{

	k *= RGPH_MURMUR32_MUL2;
	k = rgph_rotl(k, 16);
	k *= RGPH_MURMUR32_MUL3;
	h[1] ^= k;
}

static inline void
rgph_murmur32_mix2(uint32_t k, uint32_t h[/* static 4 */])
{

	k *= RGPH_MURMUR32_MUL3;
	k = rgph_rotl(k, 17);
	k *= RGPH_MURMUR32_MUL4;
	h[2] ^= k;
}

static inline void
rgph_murmur32_mix3(uint32_t k, uint32_t h[/* static 4 */])
{

	k *= RGPH_MURMUR32_MUL4;
	k = rgph_rotl(k, 18);
	k *= RGPH_MURMUR32_MUL1;
	h[3] ^= k;
}

static inline void
rgph_murmur32_mix(const uint32_t w[/*static 4 */], uint32_t h[/* static 4 */])
{
	uint32_t k;

	k = w[0] * RGPH_MURMUR32_MUL1;
	h[0] ^= rgph_rotl(k, 15) * RGPH_MURMUR32_MUL2;
	h[0] = rgph_rotl(h[0], 19);
	h[0] += h[1];
	h[0] = 5*h[0] + RGPH_MURMUR32_ADD1;

	k = w[1] * RGPH_MURMUR32_MUL2;
	h[1] ^= rgph_rotl(k, 16) * RGPH_MURMUR32_MUL3;
	h[1] = rgph_rotl(h[1], 17);
	h[1] += h[2];
	h[1] = 5*h[1] + RGPH_MURMUR32_ADD2;

	k = w[2] * RGPH_MURMUR32_MUL3;
	h[2] ^= rgph_rotl(k, 17) * RGPH_MURMUR32_MUL4;
	h[2] = rgph_rotl(h[2], 15);
	h[2] += h[3];
	h[2] = 5*h[2] + RGPH_MURMUR32_ADD3;

	k = w[3] * RGPH_MURMUR32_MUL4;
	h[3] ^= rgph_rotl(k, 18) * RGPH_MURMUR32_MUL1;
	h[3] = rgph_rotl(h[3], 13);
	h[3] += h[0];
	h[3] = 5*h[3] + RGPH_MURMUR32_ADD4;
}

static inline uint32_t
rgph_murmur32s_fmix(uint32_t h)
{

	h ^= h >> 16;
	h *= RGPH_MURMUR32S_FMIXMUL1;
	h ^= h >> 13;
	h *= RGPH_MURMUR32S_FMIXMUL2;
	h ^= h >> 16;

	return h;
}

static inline void
rgph_murmur32s_finalise(size_t len, uint32_t h[/* static 1 */])
{

	/* Note that len > UINT32_MAX is truncated. */
	h[0] ^= len;
	h[0] = rgph_murmur32s_fmix(h[0]);
}

static inline void
rgph_murmur32s_mix(uint32_t k, uint32_t h[/* static 1 */], int last)
{

	k *= RGPH_MURMUR32S_MUL1;
	k = rgph_rotl(k, 15);
	k *= RGPH_MURMUR32S_MUL2;
	h[0] ^= k;

	if (!last) {
		h[0] = rgph_rotl(h[0], 13);
		h[0] = 5*h[0] + RGPH_MURMUR32S_ADD1;
	}
}

static inline void
rgph_xxh32s_init(size_t len, uint32_t seed, uint32_t h[/* static 4 */])
{

	if (len < 4) {
		h[0] = seed + RGPH_XXH32S_PRIME5;
	} else {
		h[0] = seed + (RGPH_XXH32S_PRIME1 + RGPH_XXH32S_PRIME2);
		h[1] = seed + RGPH_XXH32S_PRIME2;
		h[2] = seed + 0;
		h[3] = seed - RGPH_XXH32S_PRIME1;
	}
}

static inline void
rgph_xxh32s_mix(const uint32_t w[/* static 4 */],
    uint32_t h[/* static 4 */])
{
	size_t i;

	for (i = 0; i < 4; i++) {
		h[i] += w[i] * RGPH_XXH32S_PRIME2;
		h[i] = rgph_rotl(h[i], 13) * RGPH_XXH32S_PRIME1;
	}
}

static inline void
rgph_xxh32s_fmix4(const uint32_t w, uint32_t h[/* static 1 */])
{

	h[0] += w * RGPH_XXH32S_PRIME3;
	h[0] = rgph_rotl(h[0], 17) * RGPH_XXH32S_PRIME4;
}

static inline void
rgph_xxh32s_fmix1(const uint8_t w, uint32_t h[/* static 1 */])
{

	h[0] += w * RGPH_XXH32S_PRIME5;
	h[0] = rgph_rotl(h[0], 11) * RGPH_XXH32S_PRIME1;
}

static inline void
rgph_xxh32s_fold(size_t len, uint32_t h[/* static 4 */])
{

	if (len >= 16) {
		h[0] = rgph_rotl(h[0], 1) +
		       rgph_rotl(h[1], 7) +
		       rgph_rotl(h[2], 12) +
		       rgph_rotl(h[3], 18);
	}

	/* Note that len > UINT32_MAX is truncated. */
	h[0] += len;
}

static inline void
rgph_xxh32s_finalise(uint32_t h[/* static 1 */])
{

	h[0] ^= h[0] >> 15;
	h[0] *= RGPH_XXH32S_PRIME2;
	h[0] ^= h[0] >> 13;
	h[0] *= RGPH_XXH32S_PRIME3;
	h[0] ^= h[0] >> 16;
}

#endif /* FILE_RGPH_HASH_IMPL_H_INCLUDED */
