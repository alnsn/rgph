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
#ifndef RGPH_GRAPH_H_INCLUDED
#define RGPH_GRAPH_H_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rgph_graph;

struct rgph_entry {
	const void *key;
	size_t keylen;
	const void *data;
	size_t datalen;
	size_t index; /* Valid index from 0 to (nkeys-1) or SIZE_MAX. */
};

typedef struct rgph_entry * (*rgph_entry_iterator_t)(void *);

struct rgph_graph *rgph_alloc_graph(size_t, int);
void rgph_free_graph(struct rgph_graph *);

int rgph_flags(struct rgph_graph *);
int rgph_rank(struct rgph_graph *);
uint32_t rgph_unassigned(struct rgph_graph *);
size_t rgph_entries(struct rgph_graph *);
size_t rgph_vertices(struct rgph_graph *);
size_t rgph_datalen_min(struct rgph_graph *);
size_t rgph_datalen_max(struct rgph_graph *);
size_t rgph_index_min(struct rgph_graph *);
size_t rgph_index_max(struct rgph_graph *);
size_t rgph_core_size(struct rgph_graph *);
unsigned long rgph_seed(struct rgph_graph *);
size_t rgph_hash_bits(struct rgph_graph *);

int rgph_build_graph(struct rgph_graph *,
    unsigned long, rgph_entry_iterator_t, void *);
int rgph_is_built(struct rgph_graph *);

int rgph_copy_edge(struct rgph_graph *, size_t, unsigned long *, size_t *);

size_t rgph_count_keys(rgph_entry_iterator_t, void *);

int rgph_find_duplicates(struct rgph_graph *,
    rgph_entry_iterator_t, void *, size_t *);

int rgph_assign(struct rgph_graph *, int);
int rgph_is_assigned(struct rgph_graph *);
const void *rgph_assignments(struct rgph_graph *, size_t *);
int rgph_copy_assignment(struct rgph_graph *, size_t, unsigned int *);

#ifdef __cplusplus
}
#endif

#endif /* !RGPH_GRAPH_H_INCLUDED */
