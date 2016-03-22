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


void
rgph_u32x4_murmur32v_data(const void *data,
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
