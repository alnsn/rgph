/*-
 * The code in this file is based on Leonid Yuriev code:
 * Portions Copyright (c) 2010-2017 Leonid Yuriev <leo@yuriev.ru>,
 * The 1Hippeus project (t1h).
 *
 * Copyright (c) 2017 Alexander Nasonov.
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


uint64_t
rgph_u64_t1ha64s_data(const void *data, size_t len, uintptr_t seed)
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

	rgph_t1ha64s_init(len, seed, h);

	if (len > 32) {
		if (down == 0) {
			for (; end - key >= 32; key += 32) {
				w[0] = rgph_read64a(&key[0]);
				w[1] = rgph_read64a(&key[8]);
				w[2] = rgph_read64a(&key[16]);
				w[3] = rgph_read64a(&key[24]);
				rgph_t1ha64s_mix(w, h);
			}
		} else {
#if !defined(UNALIGNED_READ)
			for (; end - key >= 32; key += 32) {
				rgph_read64u(key, 8 - down, &carry, w, 4);
				rgph_t1ha64s_mix(w, h);
			}
#endif
		}

		len &= 31;
		rgph_t1ha64s_fold(h);
	}

	if (down == 0) {
		size_t tail = ((len - 1) & 7) + 1;
		uint64_t mask = ~(UINT64_MAX << 1 << (tail * CHAR_BIT - 1));

		switch (len) {
		case 32: case 30: case 28: case 26:
		case 31: case 29: case 27: case 25:
			rgph_t1ha64s_fmix4(rgph_read64a(key), h);
			key += 8;
			/* FALLTHROUGH */
		case 24: case 22: case 20: case 18:
		case 23: case 21: case 19: case 17:
			rgph_t1ha64s_fmix3(rgph_read64a(key), h);
			key += 8;
			/* FALLTHROUGH */
		case 16: case 14: case 12: case 10:
		case 15: case 13: case 11: case 9:
			rgph_t1ha64s_fmix2(rgph_read64a(key), h);
			key += 8;
			/* FALLTHROUGH */
		case 8: case 6: case 4: case 2:
		case 7: case 5: case 3: case 1:
			rgph_t1ha64s_fmix1(rgph_read64a(key) & mask, h);
			break;
		}
	} else {
#if !defined(UNALIGNED_READ)
		size_t tail = ((len - 1) & 7) + 1;
		uint64_t mask = ~(UINT64_MAX << 1 << (tail * CHAR_BIT - 1));

		switch (len) {
		case 32: case 30: case 28: case 26:
		case 31: case 29: case 27: case 25:
			rgph_read64u(key, 8 - down, &carry, w, 1);
			rgph_t1ha64s_fmix4(w[0], h);
			key += 8;
			/* FALLTHROUGH */
		case 24: case 22: case 20: case 18:
		case 23: case 21: case 19: case 17:
			rgph_read64u(key, 8 - down, &carry, w, 1);
			rgph_t1ha64s_fmix3(w[0], h);
			key += 8;
			/* FALLTHROUGH */
		case 16: case 14: case 12: case 10:
		case 15: case 13: case 11: case 9:
			rgph_read64u(key, 8 - down, &carry, w, 1);
			rgph_t1ha64s_fmix2(w[0], h);
			key += 8;
			/* FALLTHROUGH */
		case 8: case 6: case 4: case 2:
		case 7: case 5: case 3: case 1:
			rgph_read64u(key, 8 - down, &carry, w, 1);
			rgph_t1ha64s_fmix1(w[0] & mask, h);
			break;
		}
#endif
	}

	return rgph_t1ha64s_finalise(h);
}
