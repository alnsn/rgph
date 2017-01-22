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

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
/* Non-public external symbols for aliasing. */
uint64_t rgph_u64_t1ha64s_data32(const void *, size_t, uintptr_t);
uint64_t rgph_u64_t1ha64s_data64(const void *, size_t, uintptr_t);
#endif


uint64_t
rgph_u64_t1ha64s_u8(uint8_t value, uintptr_t seed)
{
	uint64_t h[4];

	rgph_t1ha64s_init(sizeof(value), seed, h);

	rgph_t1ha64s_fmix1(value, h);
	return rgph_t1ha64s_finalise(h);
}

uint64_t
rgph_u64_t1ha64s_u16(uint16_t value, uintptr_t seed)
{
	uint64_t h[4];

	rgph_t1ha64s_init(sizeof(value), seed, h);

	rgph_t1ha64s_fmix1(value, h);
	return rgph_t1ha64s_finalise(h);
}

inline uint64_t
rgph_u64_t1ha64s_u32(uint32_t value, uintptr_t seed)
{
	uint64_t h[4];

	rgph_t1ha64s_init(sizeof(value), seed, h);

	rgph_t1ha64s_fmix1(value, h);
	return rgph_t1ha64s_finalise(h);
}

inline uint64_t
rgph_u64_t1ha64s_u64(uint64_t value, uintptr_t seed)
{
	uint64_t h[4];

	rgph_t1ha64s_init(sizeof(value), seed, h);

	rgph_t1ha64s_fmix1(value, h);
	return rgph_t1ha64s_finalise(h);
}

uint64_t
rgph_u64_t1ha64s_f32(float value, uintptr_t seed)
{

	return rgph_u64_t1ha64s_u32(rgph_f2u32(value), seed);
}

uint64_t
rgph_u64_t1ha64s_f64(double value, uintptr_t seed)
{

	return rgph_u64_t1ha64s_u64(rgph_d2u64(value), seed);
}

inline uint64_t
rgph_u64_t1ha64s_data32(const void *data, size_t len, uintptr_t seed)
{
	const uint64_t scale = UINT64_C(0x100000000); /* (uint64_t)val << 32 */
	const uint32_t *key = data;
	const uint32_t *end = key + len;
	size_t bytes = len * sizeof(key[0]);
	uint64_t w[4], h[4];

	rgph_t1ha64s_init(bytes, seed, h);

	if (bytes > 32) {
		for (; end - key >= 8; key += 8) {
			w[0] = htole32(key[0]) + scale * htole32(key[1]);
			w[1] = htole32(key[2]) + scale * htole32(key[3]);
			w[2] = htole32(key[4]) + scale * htole32(key[5]);
			w[3] = htole32(key[6]) + scale * htole32(key[7]);
			rgph_t1ha64s_mix(w, h);
		}

		len &= 7;
		rgph_t1ha64s_fold(h);
	}

	w[0] = 0;

	switch (len) {
	case 8: case 7:
		w[0] = htole32(key[0]) + scale * htole32(key[1]);
		key += 2;
		rgph_t1ha64s_fmix4(w[0], h);
		/* FALLTHROUGH */
	case 6: case 5:
		w[0] = htole32(key[0]) + scale * htole32(key[1]);
		key += 2;
		rgph_t1ha64s_fmix3(w[0], h);
		/* FALLTHROUGH */
	case 4: case 3:
		w[0] = htole32(key[0]) + scale * htole32(key[1]);
		key += 2;
		rgph_t1ha64s_fmix2(w[0], h);
		/* FALLTHROUGH */
	case 2: case 1:
		w[0] = htole32(key[0]);
		if ((len & 1) == 0)
			w[0] += scale * htole32(key[1]);
		rgph_t1ha64s_fmix1(w[0], h);
		break;
	}

	return rgph_t1ha64s_finalise(h);
}

inline uint64_t
rgph_u64_t1ha64s_data64(const void *data, size_t len, uintptr_t seed)
{
	const uint64_t *key = data;
	const uint64_t *end = key + len;
	size_t bytes = len * sizeof(key[0]);
	uint64_t w[4], h[4];

	rgph_t1ha64s_init(bytes, seed, h);

	if (bytes > 32) {
		for (; end - key >= 4; key += 4) {
			w[0] = htole64(key[0]);
			w[1] = htole64(key[1]);
			w[2] = htole64(key[2]);
			w[3] = htole64(key[3]);
			rgph_t1ha64s_mix(w, h);
		}

		len &= 3;
		rgph_t1ha64s_fold(h);
	}

	switch (len) {
	case 4:
		rgph_t1ha64s_fmix4(htole64(*key++), h);
		/* FALLTHROUGH */
	case 3:
		rgph_t1ha64s_fmix3(htole64(*key++), h);
		/* FALLTHROUGH */
	case 2:
		rgph_t1ha64s_fmix2(htole64(*key++), h);
		/* FALLTHROUGH */
	case 1:
		rgph_t1ha64s_fmix1(htole64(*key++), h);
		break;
	}

	return rgph_t1ha64s_finalise(h);
}

uint64_t
rgph_u64_t1ha64s_u8a(const uint8_t *key, size_t len, uintptr_t seed)
{

	return rgph_u64_t1ha64s_data(key, len, seed);
}

uint64_t
rgph_u64_t1ha64s_u16a(const uint16_t *key, size_t len, uintptr_t seed)
{
	const uint8_t *arg = rgph_unalias(const uint8_t *, key);

	return rgph_u64_t1ha64s_data(arg, len * sizeof(key[0]), seed);
}

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_t1ha64s_u32a(const uint32_t *, size_t,
    uintptr_t) __attribute__((weak,alias("rgph_u64_t1ha64s_data32")));
#else
uint64_t
rgph_u64_t1ha64s_u32a(const uint32_t *key, size_t len, uintptr_t seed)
{

	return rgph_u64_t1ha64s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_t1ha64s_u64a(const uint64_t *, size_t,
    uintptr_t) __attribute__((weak,alias("rgph_u64_t1ha64s_data64")));
#else
uint64_t
rgph_u64_t1ha64s_u64a(const uint64_t *key, size_t len, uintptr_t seed)
{

	return rgph_u64_t1ha64s_data64(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_t1ha64s_f32a(const float *, size_t,
    uintptr_t) __attribute__((weak,alias("rgph_u64_t1ha64s_data32")));
#else
uint64_t
rgph_u64_t1ha64s_f32a(const float *key, size_t len, uintptr_t seed)
{

	return rgph_u64_t1ha64s_data32(key, len, seed);
}
#endif

#if defined(WEAK_ALIASES) && !defined(__STRICT_ANSI__)
uint64_t rgph_u64_t1ha64s_f64a(const double *, size_t,
    uintptr_t) __attribute__((weak,alias("rgph_u64_t1ha64s_data64")));
#else
uint64_t
rgph_u64_t1ha64s_f64a(const double *key, size_t len, uintptr_t seed)
{

	return rgph_u64_t1ha64s_data64(key, len, seed);
}
#endif
