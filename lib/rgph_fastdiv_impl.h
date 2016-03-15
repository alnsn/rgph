/*-
 * Public Domain 2010 ridiculous_fish.
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

#ifndef FILE_RGPH_FASTDIV_IMPL_H_INCLUDED
#define FILE_RGPH_FASTDIV_IMPL_H_INCLUDED

static inline int
fls32(uint32_t n)
{
	int v;

	if (!n)
		return 0;

	v = 32;
	if ((n & 0xFFFF0000U) == 0) {
		n <<= 16;
		v -= 16;
	}
	if ((n & 0xFF000000U) == 0) {
		n <<= 8;
		v -= 8;
	}
	if ((n & 0xF0000000U) == 0) {
		n <<= 4;
		v -= 4;
	}
	if ((n & 0xC0000000U) == 0) {
		n <<= 2;
		v -= 2;
	}
	if ((n & 0x80000000U) == 0) {
		n <<= 1;
		v -= 1;
	}
	return v;
}

#endif /* FILE_RGPH_FASTDIV_IMPL_H_INCLUDED */
