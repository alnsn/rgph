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

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include "rgph_fastdiv.h"

/*
 * Computes "magic info" for performing unsigned division by a fixed
 * positive integer D.  The type 'uint' is assumed to be defined as
 * an unsigned integer type large enough to hold both the dividend
 * and the divisor. num_bits can be set appropriately if n is known
 * to be smaller than the largest uint; if this is not known then
 * pass (sizeof(uint) * CHAR_BIT) for num_bits.
 *
 * Assume we have a hardware register of width UINT_BITS, a known
 * constant D which is not zero and not a power of 2, and a variable
 * n of width num_bits (which may be up to UINT_BITS). To emit code
 * for n/d, use one of the two following sequences (here >>> refers
 * to a logical bitshift):
 *
 *     m = compute_unsigned_magic_info(D, num_bits)
 *     if m.pre_shift > 0: emit("n >>>= m.pre_shift")
 *     if m.increment: emit("n = saturated_increment(n)")
 *     emit("result = (m.multiplier * n) >>> UINT_BITS")
 *     if m.post_shift > 0: emit("result >>>= m.post_shift")
 *
 * or
 *
 *     m = compute_unsigned_magic_info(D, num_bits)
 *     if m.pre_shift > 0: emit("n >>>= m.pre_shift")
 *     emit("result = m.multiplier * n")
 *     if m.increment: emit("result = result + m.multiplier")
 *     emit("result >>>= UINT_BITS")
 *     if m.post_shift > 0: emit("result >>>= m.post_shift")
 *
 * The shifts by UINT_BITS may be "free" if the high half of the full
 * multiply is put in a separate register.
 *
 * saturated_increment(n) means "increment n unless it would wrap to 0," i.e.
 *    if n == (1 << UINT_BITS)-1: result = n
 *    else: result = n+1
 * A common way to implement this is with the carry bit. For example, on x86:
 *     add 1
 *     sbb 0
 *
 * Some invariants:
 *   1: At least one of pre_shift and increment is zero
 *   2: multiplier is never zero
 *
 * This code incorporates the "round down" optimization per ridiculous_fish.
 */
struct magicu_info {
	uint32_t multiplier; /* the "magic number" multiplier */
	unsigned int pre_shift; /* shift for the dividend before multiplying */
	unsigned int post_shift; /* shift for the dividend after multiplying */
	int increment; /* 0 or 1; if set then increment the numerator,
	                * using one of the two strategies */
};

static struct magicu_info
compute_unsigned_magic_info(unsigned int D, unsigned num_bits);

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

/*
 * Public Domain 2010 ridiculous_fish.
 * This is free and unencumbered software. Any copyright is dedicated
 * to the Public Domain.
 */
static struct magicu_info
compute_unsigned_magic_info(uint32_t D, unsigned int num_bits)
{
	/* The eventual result. */
	struct magicu_info result;

	/* Bits in a uint32_t. */
	const unsigned int UINT_BITS = sizeof(uint32_t) * CHAR_BIT;

	/*
	 * The extra shift implicit in the difference
	 * between UINT_BITS and num_bits.
	 */
	const unsigned int extra_shift = UINT_BITS - num_bits;

	/*
	 * The initial power of 2 is one less than
	 * the first one that can possibly work.
	 */
	const uint32_t initial_power_of_2 = 1u << (UINT_BITS-1);

	/* The remainder and quotient of our power of 2 divided by d. */
	uint32_t quotient = initial_power_of_2 / D;
	uint32_t remainder = initial_power_of_2 % D;

	/* ceil(log_2 D) */
	unsigned ceil_log_2_D = fls32(D) - 1;

	/* The magic info for the variant "round down" algorithm. */
	uint32_t down_multiplier = 0;
	unsigned down_exponent = 0;
	int has_magic_down = 0;

	unsigned int exponent;

	/* The numerator must fit in a uint32_t. */
	assert(num_bits > 0 && num_bits <= sizeof(uint32_t) * CHAR_BIT);

	/* D must be larger than zero and not a power of 2 */
	assert(D & (D-1));

	/*
	 * Begin a loop that increments the exponent,
	 * until we find a power of 2 that works.
	 */
	for (exponent = 0; ; exponent++) {
		/*
		 * Quotient and remainder is from previous exponent;
		 * compute it for this exponent.
		 */
		if (remainder >= D - remainder) {
			/* Doubling remainder will wrap around D. */
			quotient = quotient * 2 + 1;
			remainder = remainder * 2 - D;
		} else {
			/* Remainder will not wrap. */
			quotient = quotient * 2;
			remainder = remainder * 2;
		}

		/*
		 * We're done if this exponent works for the round_up
		 * algorithm. Note that exponent may be larger than
		 * the maximum shift supported, so the check for >=
		 * ceil_log_2_D is critical.
		 */
		if ((exponent + extra_shift >= ceil_log_2_D) ||
		    (D - remainder) <= (1u << (exponent + extra_shift)))
			break;

		/*
		 * Set magic_down if we have not set it yet and this
		 * exponent works for the round_down algorithm.
		 */
		if (!has_magic_down &&
		    remainder <= (1u << (exponent + extra_shift))) {
			has_magic_down = 1;
			down_multiplier = quotient;
			down_exponent = exponent;
		}
	}

	if (exponent < ceil_log_2_D) {
		/* magic_up is efficient. */
		result.multiplier = quotient + 1;
		result.pre_shift = 0;
		result.post_shift = exponent;
		result.increment = 0;
	} else if (D & 1) {
		/* Odd divisor, so use magic_down, which must have been set. */
		assert(has_magic_down);
		result.multiplier = down_multiplier;
		result.pre_shift = 0;
		result.post_shift = down_exponent;
		result.increment = 1;
	} else {
		/* Even divisor, so use a prefix-shifted dividend. */
		unsigned pre_shift = 0;
		uint32_t shifted_D = D;

		while ((shifted_D & 1) == 0) {
			shifted_D >>= 1;
			pre_shift += 1;
		}

		result = compute_unsigned_magic_info(shifted_D,
		    num_bits - pre_shift);
		/* Expect no increment or pre_shift in this path. */
		assert(result.increment == 0 && result.pre_shift == 0);
		result.pre_shift = pre_shift;
	}

	return result;
}

void
rgph_fastdiv_prepare(uint32_t div, uint32_t *m,
    uint8_t *s1, uint8_t *s2, int *increment)
{
	struct magicu_info mi;
	uint64_t mt;
	int l;

	assert(div > 0);

	if (increment == NULL) {
		/* fast_divide32(3) from NetBSD. */
		l = fls32(div - 1);
		mt = ((UINT64_C(1) << l) - div) << 32;
		*m = (uint32_t)(mt / div + 1);
		*s1 = (l > 1) ? 1 : l;
		*s2 = (l == 0) ? 0 : l - 1;
	} else {
		mi = compute_unsigned_magic_info(div, sizeof(div) * CHAR_BIT);
		*m = mi.multiplier;
		*s1 = mi.pre_shift;
		*s2 = mi.post_shift;
		*increment = mi.increment;
	}
}
