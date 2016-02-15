/*-
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
#ifndef RGPH_DEFS_H_INCLUDED
#define RGPH_DEFS_H_INCLUDED

/* Return values of rgph functions. */
#define RGPH_SUCCESS 0
#define RGPH_INVAL  -1 /* EINVAL. */
#define RGPH_RANGE  -2 /* ERANGE. */
#define RGPH_NOMEM  -3 /* ENOMEM. */
#define RGPH_AGAIN  -4 /* Graph has a cycle or rgph_find_duplicates() failed. */
#define RGPH_NOKEY  -5 /* Iterator returned no key. */

/*
 * (RGPH_HASH_DEFAULT | RGPH_RANK3 | RGPH_ALGO_DEFAULT |
 *     RGPH_ROUND_DEFAULT | RGPH_INDEX_DEFAULT)
 */
#define	RGPH_DEFAULT       0

#define	RGPH_HASH_MASK      0xff
#define	RGPH_HASH_DEFAULT   0
#define	RGPH_HASH_JENKINS2  1
#define	RGPH_HASH_JENKINS3  2 /* NIY */
#define	RGPH_HASH_JENKINS3S 3 /* NIY */
#define	RGPH_HASH_MURMUR32  4
#define	RGPH_HASH_MURMUR32S 5
#define	RGPH_HASH_XXH32S    6
#define	RGPH_HASH_XXH64S    7

#define	RGPH_RANK_MASK     0x100
#define	RGPH_RANK2         0x100
#define	RGPH_RANK3         0

#define	RGPH_ALGO_MASK     0x600
#define	RGPH_ALGO_DEFAULT  0
#define	RGPH_ALGO_CHM      0x200
#define	RGPH_ALGO_BDZ      0x400

#define RGPH_ROUND_MASK    0x800
#define RGPH_ROUND_DEFAULT 0
#define RGPH_ROUND_POW2    0x800

#define	RGPH_INDEX_MASK    0x3000
#define	RGPH_INDEX_DEFAULT 0
#define	RGPH_INDEX_XXX     0x1000

#endif /* !RGPH_DEFS_H_INCLUDED */
