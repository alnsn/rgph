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


inline uint32_t
rgph_u32_murmur32s_data(const void *data, size_t len, uint32_t seed)
{
	const uint8_t *key = data;
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

void
rgph_u8x4_murmur32s_data(const void *key,
    size_t len, uint32_t seed, uint8_t *h8)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data(key, len, seed);
	h8[0] = (uint8_t)(h >> 24);
	h8[1] = (uint8_t)(h >> 16);
	h8[2] = (uint8_t)(h >> 8);
	h8[3] = (uint8_t)h;
}

void
rgph_u16x2_murmur32s_data(const void *data,
    size_t len, uint32_t seed, uint16_t *h16)
{
	uint32_t h;

	h = rgph_u32_murmur32s_data(data, len, seed);
	h16[0] = (uint16_t)(h >> 16);
	h16[1] = (uint16_t)h;
}
