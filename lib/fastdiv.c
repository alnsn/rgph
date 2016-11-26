/*-
 * Copyright (c) 2016 Alexander Nasonov.
 * Copyright (c) 2007, 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas and Joerg Sonnenberger.
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

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include "rgph_fastdiv.h"
#include "rgph_bitops.h"

void
rgph_fastdiv_prepare(uint32_t div, uint32_t *m,
    uint8_t *s1, uint8_t *s2, int branchless)
{
	/* fast_divide32(3) from NetBSD. */
	const int l = fls32(div - 1);
	const uint64_t nextpow2 = UINT64_C(1) << l;
	uint32_t rem, magic = ((nextpow2 - div) << 32) / div;

	*m = magic + 1;
	*s1 = (l > 1) ? 1 : l;
	*s2 = (l == 0) ? 0 : l - 1;

	if (branchless)
		return;

	if (magic == 0) { /* div == nextpow2 */
		*m = 0;
		*s1 -= 1;
		*s2 += 1;
	} else {
		/*
		 * See libdivide_internal_u32_gen()
		 * by ridiculous_fish for clues.
		 */
		magic = (magic + UINT64_C(0x100000000)) / 2;
		rem = (nextpow2 << 31) - magic * div;
		assert(rem > 0 && rem < div);
		if (rem > div - (nextpow2 / 2)) {
			*m = magic + 1;
			*s1 -= 1;
		}
	}
}
