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

#ifdef __cplusplus
extern "C" {
#endif

/* Jenkins 2 generic x3 hash for any data. */
void rgph_u32x3_jenkins2v_data(const void *, size_t, uint32_t, uint32_t *);

/* Jenkins 2 x3 hashes for fixed width types. */
void rgph_u32x3_jenkins2v_u8 (uint8_t,  uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u16(uint16_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u32(uint32_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u64(uint64_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_f32(float,    uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_f64(double,   uint32_t, uint32_t *);

/* Jenkins 2 x3 hashes for arrays. */
void rgph_u32x3_jenkins2v_u8a(const uint8_t *,   size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u16a(const uint16_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u32a(const uint32_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_u64a(const uint64_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_f32a(const float *,    size_t, uint32_t, uint32_t *);
void rgph_u32x3_jenkins2v_f64a(const double *,   size_t, uint32_t, uint32_t *);


/* 32bit Murmur 3 generic x4 hash for any data. */
void rgph_u32x4_murmur32v_data(const void *, size_t, uint32_t, uint32_t *);

/* 32bit Murmur 3 x4 hashes for fixed width types. */
void rgph_u32x4_murmur32v_u8 (uint8_t,  uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u16(uint16_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u32(uint32_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u64(uint64_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_f32(float,    uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_f64(double,   uint32_t, uint32_t *);

/* 32bit Murmur 3 x4 hashes for arrays. */
void rgph_u32x4_murmur32v_u8a(const uint8_t *,   size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u16a(const uint16_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u32a(const uint32_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_u64a(const uint64_t *, size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_f32a(const float *,    size_t, uint32_t, uint32_t *);
void rgph_u32x4_murmur32v_f64a(const double *,   size_t, uint32_t, uint32_t *);


/* 32bit Murmur 3 (short) generic scalar hash for any data. */
uint32_t rgph_u32_murmur32s_data(const void *, size_t, uint32_t);

/* 32bit Murmur 3 (short) scalar 32bit hashes for fixed width types. */
uint32_t rgph_u32_murmur32s_u8 (uint8_t,  uint32_t);
uint32_t rgph_u32_murmur32s_u16(uint16_t, uint32_t);
uint32_t rgph_u32_murmur32s_u32(uint32_t, uint32_t);
uint32_t rgph_u32_murmur32s_u64(uint64_t, uint32_t);
uint32_t rgph_u32_murmur32s_f32(float,  uint32_t);
uint32_t rgph_u32_murmur32s_f64(double, uint32_t);

/* 32bit Murmur 3 (short) scalar 32bit hashes for arrays. */
uint32_t rgph_u32_murmur32s_u8a (const uint8_t *,  size_t, uint32_t);
uint32_t rgph_u32_murmur32s_u16a(const uint16_t *, size_t, uint32_t);
uint32_t rgph_u32_murmur32s_u32a(const uint32_t *, size_t, uint32_t);
uint32_t rgph_u32_murmur32s_u64a(const uint64_t *, size_t, uint32_t);
uint32_t rgph_u32_murmur32s_f32a(const float *,  size_t, uint32_t);
uint32_t rgph_u32_murmur32s_f64a(const double *, size_t, uint32_t);


/* 32bit xxHash (short) generic scalar hash for any data. */
uint32_t rgph_u32_xxh32s_data(const void *, size_t, uint32_t);

/* 32bit xxHash (short) scalar 32bit hashes for fixed width types. */
uint32_t rgph_u32_xxh32s_u8 (uint8_t,  uint32_t);
uint32_t rgph_u32_xxh32s_u16(uint16_t, uint32_t);
uint32_t rgph_u32_xxh32s_u32(uint32_t, uint32_t);
uint32_t rgph_u32_xxh32s_u64(uint64_t, uint32_t);
uint32_t rgph_u32_xxh32s_f32(float,  uint32_t);
uint32_t rgph_u32_xxh32s_f64(double, uint32_t);

/* 32bit xxHash (short) scalar 32bit hashes for arrays. */
uint32_t rgph_u32_xxh32s_u8a (const uint8_t *,  size_t, uint32_t);
uint32_t rgph_u32_xxh32s_u16a(const uint16_t *, size_t, uint32_t);
uint32_t rgph_u32_xxh32s_u32a(const uint32_t *, size_t, uint32_t);
uint32_t rgph_u32_xxh32s_u64a(const uint64_t *, size_t, uint32_t);
uint32_t rgph_u32_xxh32s_f32a(const float *,  size_t, uint32_t);
uint32_t rgph_u32_xxh32s_f64a(const double *, size_t, uint32_t);


/* 64bit xxHash (short) generic scalar hash for any data. */
uint64_t rgph_u64_xxh64s_data(const void *, size_t, uint32_t);

/* 64bit xxHash (short) scalar 64bit hashes for fixed width types. */
uint64_t rgph_u64_xxh64s_u8 (uint8_t,  uint32_t);
uint64_t rgph_u64_xxh64s_u16(uint16_t, uint32_t);
uint64_t rgph_u64_xxh64s_u32(uint32_t, uint32_t);
uint64_t rgph_u64_xxh64s_u64(uint64_t, uint32_t);
uint64_t rgph_u64_xxh64s_f32(float,  uint32_t);
uint64_t rgph_u64_xxh64s_f64(double, uint32_t);

/* 64bit xxHash (short) scalar 64bit hashes for arrays. */
uint64_t rgph_u64_xxh64s_u8a (const uint8_t *,  size_t, uint32_t);
uint64_t rgph_u64_xxh64s_u16a(const uint16_t *, size_t, uint32_t);
uint64_t rgph_u64_xxh64s_u32a(const uint32_t *, size_t, uint32_t);
uint64_t rgph_u64_xxh64s_u64a(const uint64_t *, size_t, uint32_t);
uint64_t rgph_u64_xxh64s_f32a(const float *,  size_t, uint32_t);
uint64_t rgph_u64_xxh64s_f64a(const double *, size_t, uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* !FILE_RGPH_HASH_H_INCLUDED */
