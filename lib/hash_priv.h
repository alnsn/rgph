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
#ifndef FILE_RGPH_HASH_PRIV_H_INCLUDED
#define FILE_RGPH_HASH_PRIV_H_INCLUDED

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

#include <stdint.h>
#include <limits.h>

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

#define RGPH_MURMUR32_MIX(k1, k2, k3, k4, h)                              \
	k1 *= RGPH_MURMUR32_MUL1; k1 = rgph_rotl(k1, 15);                 \
	k1 *= RGPH_MURMUR32_MUL2; h[0] ^= k1; h[0] = rgph_rotl(h[0], 19); \
	h[0] += h[1]; h[0] = 5*h[0] + RGPH_MURMUR32_ADD1;                 \
	k2 *= RGPH_MURMUR32_MUL2; k2 = rgph_rotl(k2, 16);                 \
	k2 *= RGPH_MURMUR32_MUL3; h[1] ^= k2; h[1] = rgph_rotl(h[1], 17); \
	h[1] += h[2]; h[1] = 5*h[1] + RGPH_MURMUR32_ADD2;                 \
	k3 *= RGPH_MURMUR32_MUL3; k3 = rgph_rotl(k3, 17);                 \
	k3 *= RGPH_MURMUR32_MUL4; h[2] ^= k3; h[2] = rgph_rotl(h[2], 15); \
	h[2] += h[3]; h[2] = 5*h[2] + RGPH_MURMUR32_ADD3;                 \
	k4 *= RGPH_MURMUR32_MUL4; k4 = rgph_rotl(k4, 18);                 \
	k4 *= RGPH_MURMUR32_MUL1; h[3] ^= k4; h[3] = rgph_rotl(h[3], 13); \
	h[3] += h[0]; h[3] = 5*h[3] + RGPH_MURMUR32_ADD4;

/*
 * Rotate left and right.
 */
#define rgph_rotl(x, l) (((x) << (l)) | ((x) >> (CHAR_BIT * sizeof(x) - (l))))
#define rgph_rotr(x, r) (((x) >> (r)) | ((x) << (CHAR_BIT * sizeof(x) - (r))))

/*
 * Read 32bit word from aligned pointer in little-endian order.
 */
#define rgph_read32a(aligned) htole32(*((const uint32_t *)(aligned)))

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
f2u32(float f)
{
	/* This conversion isn't well defined but it's faster than memcpy(3). */
	union {
		float f;
		uint64_t u;
	} u = { f };

	return u.u;
}

static inline uint64_t
d2u64(double d)
{
	/* Weird half little-endian half big-endian FP isn't supported. */
	union {
		double d;
		uint64_t u;
	} u = { d };

	return u.u;
}

#endif /* FILE_RGPH_HASH_PRIV_H_INCLUDED */
