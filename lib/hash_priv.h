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
#else
#include <sys/endian.h>
#endif

#include <stdint.h>
#include <limits.h>

/*
 * Rotate left and right.
 */
#define rotl(x, l) (((x) << (l)) | ((x) >> (CHAR_BIT * sizeof(x) - (l))))
#define rotr(x, r) (((x) >> (r)) | ((x) << (CHAR_BIT * sizeof(x) - (r))))

/*
 * Read 4 bytes from buf in little-endian order.
 */
#if defined(RGPH_UNALIGNED_READ)
#define read32(buf) htole32(*((const uint32_t *)(buf)))
#else
#define read32(buf) ( \
	((uint32_t)((const uint8_t *)(buf))[3] << 24) | \
	((uint32_t)((const uint8_t *)(buf))[2] << 16) | \
	((uint32_t)((const uint8_t *)(buf))[1] << 8)  | \
	((uint32_t)((const uint8_t *)(buf))[0]))
#endif

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
