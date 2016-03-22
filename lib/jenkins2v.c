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


void
rgph_u32x3_jenkins2v_data(const void *data,
    size_t len, uint32_t seed, uint32_t * restrict h)
{
	const uint8_t * restrict key = data;
	const uint8_t *end = key + len;
	int n = 0;
#if defined(UNALIGNED_READ)
	const int down = 0;
#else
	uint32_t w[3];
	const int down = ((uintptr_t)key) & 3;
	uint32_t carry = len > 0 && down != 0 ? rgph_read32a(key - down) : 0;
#endif

	h[0] = RGPH_JENKINS2_SEED1;
	h[1] = RGPH_JENKINS2_SEED2;
	h[2] = seed;

	if (down == 0) {
		for (; end - key >= 12; key += 12) {
			h[0] += rgph_read32a(&key[0]);
			h[1] += rgph_read32a(&key[4]);
			h[2] += rgph_read32a(&key[8]);
			rgph_jenkins2_mix(h);
		}

		if (end - key >= 4) {
			h[n++] += rgph_read32a(&key[0]);
			key += 4;
		}
		if (end - key >= 4) {
			h[n++] += rgph_read32a(&key[0]);
			key += 4;
		}
	} else {
#if !defined(UNALIGNED_READ)
		for (; end - key >= 12; key += 12) {
			rgph_read32u(key, 4 - down, &carry, w, 3);
			h[0] += w[0];
			h[1] += w[1];
			h[2] += w[2];
			rgph_jenkins2_mix(h);
		}

		if (end - key >= 4) {
			rgph_read32u(key, 4 - down, &carry, w, 1);
			h[n++] += w[0];
			key += 4;
		}
		if (end - key >= 4) {
			rgph_read32u(key, 4 - down, &carry, w, 1);
			h[n++] += w[0];
			key += 4;
		}
#endif
	}

	switch (end - key) {
	case 3:
		h[n] += (uint32_t)key[2] << 16; /* FALLTHROUGH */
	case 2:
		h[n] += (uint32_t)key[1] << 8;  /* FALLTHROUGH */
	case 1:
		h[n] += (uint32_t)key[0];       /* FALLTHROUGH */
	case 0:
		h[2] += len;
		rgph_jenkins2_mix(h);
	}
}
