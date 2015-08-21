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
#ifndef FILE_RGPH_HASH_H_INCLUDED
#define FILE_RGPH_HASH_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#define RGPH_JENKINS2_SEED1 UINT32_C(0x9e3779b9)
#define RGPH_JENKINS2_SEED2 UINT32_C(0x9e3779b9)

#ifdef __cplusplus
extern "C" {
#endif

/* Generic scalar and x3 hashes for any data. */
uint32_t rgph_u32_jenkins2_data(const void *, size_t, uint32_t);
uint64_t rgph_u64_jenkins2_data(const void *, size_t, uint32_t);
void rgph_u32x3_jenkins2_data(const void *, size_t, uint32_t, uint32_t *);

/* Scalar 32bit hashes for fixed width types. */
uint32_t rgph_u32_jenkins2_u8(uint8_t,   uint32_t);
uint32_t rgph_u32_jenkins2_u16(uint16_t, uint32_t);
uint32_t rgph_u32_jenkins2_u32(uint32_t, uint32_t);
uint32_t rgph_u32_jenkins2_u64(uint64_t, uint32_t);
uint32_t rgph_u32_jenkins2_f32(float,    uint32_t);
uint32_t rgph_u32_jenkins2_f64(double,   uint32_t);

/* Scalar 64bit hashes for fixed width types. */
uint64_t rgph_u64_jenkins2_u8 (uint8_t,  uint32_t);
uint64_t rgph_u64_jenkins2_u16(uint16_t, uint32_t);
uint64_t rgph_u64_jenkins2_u32(uint32_t, uint32_t);
uint64_t rgph_u64_jenkins2_u64(uint64_t, uint32_t);
uint64_t rgph_u64_jenkins2_f32(float,    uint32_t);
uint64_t rgph_u64_jenkins2_f64(double,   uint32_t);

/* Vector 32bit x3 hashes for fixed width types. */
void rgph_u32x3_jenkins2_u8 (uint8_t,  uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u16(uint16_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u32(uint32_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u64(uint64_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_f32(float,    uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_f64(double,   uint32_t, uint32_t *);

/* Vector 32bit x3 hashes for arrays. */
void rgph_u32x3_jenkins2_u8a(const uint8_t *,   size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u16a(const uint16_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u32a(const uint32_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_u64a(const uint64_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_f32a(const float *,    size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2_f64a(const double *,   size_t, uint32_t, uint32_t *);

#ifdef __cplusplus
}
#endif

#endif /* !FILE_RGPH_HASH_H_INCLUDED */
