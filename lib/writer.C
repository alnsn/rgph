/*-
 * Copyright (c) 2015 Alexander Nasonov.
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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rgph_defs.h"
#include "rgph_hash.h"
#include "rgph_graph.h"

// Min. data_width size for building a graph. Setting it to 1 or 2
// may save you some space for smaller graphs but beware of subtleties
// of integral promotion rules.
#define MIN_WIDTH_BUILD 4

namespace {

template<bool C> struct bool_selector {};

/*
 * The algorithm below is based on paper
 * Cache-Oblivious Peeling of Random Hypergraphs by Djamal Belazzougui,
 * Paolo Boldi, Giuseppe Ottaviano, Rossano Venturini, and Sebastiano
 * Vigna.
 * http://zola.di.unipi.it/rossano/wp-content/papercite-data/pdf/dcc14.pdf
 */
template<class T, int R>
struct edge {
	T verts[R]; // v0, v1 (and v2, if R==3).
	T index; // XXX
};

/*
 * Data type for a valid oriented edge (v0, v1, v2), v1 < v2.
 * The first vertex v0 is implicit and is determined by an index
 * of the corresponding element in the oedges array.
 * If the degree of v0 is greater than 1, other members don't make
 * sense because they're a result of XORing multiple random values.
 */
template<class T, int R>
struct oedge {
	T overts[R-1]; // v1 (and v2, if R==3).
	T degree;      // Degree of v0.
	T edge;
};

struct entry_iterator {
	rgph_entry_iterator_t iter;
	void *state;
	struct rgph_entry *cur;

	entry_iterator(rgph_entry_iterator_t i, void *s)
		: iter(i)
		, state(s)
		, cur(iter(state))
	{}

	entry_iterator()
		: iter(NULL)
		, state(NULL)
		, cur(NULL)
	{}

	void operator++() {
		cur = iter(state);
	}

	struct rgph_entry *operator->() {
		assert(cur != NULL);
		return cur;
	}

	const rgph_entry &operator*() {
		assert(cur != NULL);
		return *cur;
	}
};

// Good enough to detect past-the-end.
inline bool
operator!=(const entry_iterator &a, const entry_iterator &b)
{

	return a.cur != b.cur;
}

template<class T, class S, class H>
struct hash {
	typedef void (*func_t)(const void *, size_t, S, H *);
	func_t func;
	T hashes[3];
	S seed;

	inline hash(func_t f, S s) : func(f), seed(s) {}

	inline const T *
	operator()(const void *key, size_t keylen) {
		bool_selector<(sizeof(T) < sizeof(H))> selector;
		return this->hashit(selector, key, keylen);
	}

	inline const T *
	hashit(bool_selector<false>, const void *key, size_t keylen) {
		// H == T or H is smaller. The latter will not compile.
		func(key, keylen, seed, hashes);
		return hashes;
	}
	inline const T *
	hashit(bool_selector<true>, const void *key, size_t keylen) {
		// T is smaller than H.
		H h[3];
		func(key, keylen, seed, h);
		hashes[0] = (T)h[0];
		hashes[1] = (T)h[1];
		hashes[2] = (T)h[2];
		return hashes;
	}
};

template<class T, class S, class H>
inline hash<T,S,H>
make_hash(void (*func)(const void *, size_t, S, H *), unsigned long seed)
{

	return hash<T,S,H>(func, seed);
}

template<class T>
inline void
add_remove_oedge(oedge<T,2> *oedges, int delta, T e, T v0, T v1)
{

	oedges[v0].overts[0] ^= v1;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class T>
inline void
remove_edge(oedge<T,2> *oedges, T e, T v0, T v1)
{

	return add_remove_oedge(oedges, -1, e, v0, v1);
}

template<class T>
inline void
add_remove_oedge(oedge<T,3> *oedges, int delta, T e, T v0, T v1, T v2)
{

	oedges[v0].overts[v1 < v2 ? 0 : 1] ^= v1;
	oedges[v0].overts[v1 < v2 ? 1 : 0] ^= v2;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class T>
inline void
remove_oedge(oedge<T,2> *oedges, T e, T v0, T v1)
{

	return add_remove_oedge(oedges, -1, e, v0, v1);
}

template<class T>
inline void
remove_oedge(oedge<T,3> *oedges, T e, T v0, T v1, T v2)
{

	return add_remove_oedge(oedges, -1, e, v0, v1, v2);
}

template<class T>
inline void
add_edge(oedge<T,2> *oedges, T e, const T *verts)
{

	add_remove_oedge(oedges, 1, e, verts[0], verts[1]);
	add_remove_oedge(oedges, 1, e, verts[1], verts[0]);
}

template<class T>
inline void
add_edge(oedge<T,3> *oedges, T e, const T *verts)
{

	add_remove_oedge(oedges, 1, e, verts[0], verts[1], verts[2]);
	add_remove_oedge(oedges, 1, e, verts[1], verts[0], verts[2]);
	add_remove_oedge(oedges, 1, e, verts[2], verts[0], verts[1]);
}

template<class T>
inline size_t
remove_vertex(oedge<T,2> *oedges, T v0, T *order, size_t end)
{

	if (oedges[v0].degree == 1) {
		const T e = oedges[v0].edge;
		const T v1 = oedges[v0].overts[0];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0);
		order[--end] = e;
	}

	return end;
}

template<class T>
inline size_t
remove_vertex(oedge<T,3> *oedges, T v0, T *order, size_t end)
{

	if (oedges[v0].degree == 1) {
		const T e = oedges[v0].edge;
		const T v1 = oedges[v0].overts[0];
		const T v2 = oedges[v0].overts[1];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0, v2);
		remove_oedge(oedges, e, v2, v0, v1);
		order[--end] = e;
	}

	return end;
}

template<class Iter, class Hash, class T, int R>
bool
init_graph(Iter keys, Iter keys_end, Hash hash,
    edge<T,R> *edges, size_t nkeys, oedge<T,R> *oedges, size_t nverts)
{
	// partsz is a partition size of an R-partite R-graph.
	const T partsz = nverts / R;
	assert(partsz > 1 && (nverts % R) == 0);

	T e = 0;
	for (; e < nkeys && keys != keys_end; ++e, ++keys) {
		const rgph_entry &ent = *keys;
		const T *verts = hash(ent.key, ent.keylen);
		for (T r = 0; r < R; ++r)
			edges[e].verts[r] = (verts[r] % partsz) + r * partsz;
		add_edge(oedges, e, edges[e].verts);
	}

	return e == nkeys;
}

template<class T, int R>
size_t
peel_graph(edge<T,R> *edges, size_t nkeys,
    oedge<T,R> *oedges, size_t nverts, T *order)
{
	size_t end = nkeys;

	for (T v0 = 0; v0 < nverts; ++v0)
		end = remove_vertex(oedges, v0, order, end);

	for (size_t i = nkeys; i > 0 && i > end; --i) {
		const edge<T,R> &e = edges[order[i-1]];
		for (size_t r = 0; r < R; ++r)
			end = remove_vertex(oedges, e.verts[r], order, end);
	}

	return end;
}

inline size_t
data_width(size_t size, size_t min_width)
{

	if (size <= 0xffu && min_width <= 1)
		return 1;
	else if (size <= 0xffffu && min_width <= 2)
		return 2;
	else if (size <= 0xffffffffu && min_width <= 4)
		return 4;
	else
		return 0;
}

template<int R>
inline size_t
edge_size_rank(size_t nkeys, size_t min_width)
{

	switch (data_width(nkeys, min_width)) {
		case 1: return sizeof(edge<uint8_t,R>);
		case 2: return sizeof(edge<uint16_t,R>);
		case 4: return sizeof(edge<uint32_t,R>);
		default: return 0;
	}
}

inline size_t
edge_size(int rank, size_t nkeys, size_t min_width)
{

	switch (rank) {
		case 2: return edge_size_rank<2>(nkeys, min_width);
		case 3: return edge_size_rank<3>(nkeys, min_width);
		default: return 0;
	}
}

template<int R>
inline size_t
oedge_size_rank(size_t nverts, size_t min_width)
{

	switch (data_width(nverts, min_width)) {
		case 1: return sizeof(oedge<uint8_t,R>);
		case 2: return sizeof(oedge<uint16_t,R>);
		case 4: return sizeof(oedge<uint32_t,R>);
		default: return 0;
	}
}

inline size_t
oedge_size(int rank, size_t nverts, size_t min_width)
{

	switch (rank) {
		case 2: return oedge_size_rank<2>(nverts, min_width);
		case 3: return oedge_size_rank<3>(nverts, min_width);
		default: return 0;
	}
}

inline int
graph_rank(unsigned int flags)
{

	return (flags & RGPH_RANK_MASK) == RGPH_RANK2 ? 2 : 3;
}

inline size_t
round_up(size_t n, size_t r)
{

	assert(n <= SIZE_MAX - (r - 1));
	return (n + (r - 1)) / r * r;
}

} // namespace

struct rgph_graph {
	size_t nkeys;
	size_t nverts;
	void *order;
	void *edges;
	void *oedges;
	unsigned int flags;
};

enum {
	PUBLIC_FLAGS = 0x3ff,
	ZEROED = 0x40000000,
	BUILT  = 0x20000000
};

template<class T, int R>
static int
build_graph(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, unsigned long seed)
{
	typedef edge<T,R> edge_t;
	typedef oedge<T,R> oedge_t;

	T *order = (T *)g->order;
	edge_t *edges = (edge_t *)g->edges;
	oedge_t *oedges = (oedge_t *)g->oedges;
	entry_iterator keys_start(keys, state), keys_end;

	g->flags &= ~BUILT;

	switch (g->flags & RGPH_HASH_MASK) {
		case RGPH_HASH_JENKINS2:
			if (!init_graph(keys_start, keys_end,
			    make_hash<T>(&rgph_u32x3_jenkins2_data, seed),
			    edges, g->nkeys, oedges, g->nverts)) {
			    goto einval;
			}
			break;
		case RGPH_HASH_DEFAULT:
		case RGPH_HASH_JENKINS3: // XXX implement jenkins3
		default:
		einval:
			errno = EINVAL;
			return -1;
	}

	g->flags |= BUILT;

	return peel_graph(edges, g->nkeys, oedges, g->nverts, order);
}

extern "C"
void
rgph_free_graph(struct rgph_graph *g)
{

	free(g->oedges);
	free(g->edges);
	free(g->order);
	free(g);
}

extern "C"
struct rgph_graph *
rgph_alloc_graph(size_t nkeys, int flags)
{
	struct rgph_graph *g;
	size_t maxkeys, nverts, esz, osz;
	int save_errno;
	int r;

	if (flags & ~PUBLIC_FLAGS) {
		errno = EINVAL;
		return NULL;
	}

	r = graph_rank(flags);
	maxkeys = (r == 2) ? 0x78787877u : 0xcccccccau;

	if (nkeys == 0 || nkeys > maxkeys) {
		errno = ERANGE;
		return NULL;
	}

	nverts = (r == 2) ? 2 * nkeys + (nkeys + 7) / 8
	                  : 1 * nkeys + (nkeys + 3) / 4;
	nverts = round_up(nverts, r);
	if (nverts < 24)
		nverts = 24;

	assert(nverts > nkeys);

	esz = edge_size(r, nverts, MIN_WIDTH_BUILD); // data_width(nverts) !
	osz = oedge_size(r, nverts, MIN_WIDTH_BUILD);

	if (esz == 0 || osz == 0) {
		errno = EINVAL;
		return NULL;
	}

	g = (struct rgph_graph *)calloc(sizeof(*g), 1);
	if (g == NULL)
		return NULL;

	g->nkeys = 0;
	g->nverts = 0;
	g->order = NULL;
	g->edges = NULL;
	g->oedges = NULL;
	g->flags = flags;

	g->order = calloc(esz, nkeys);
	if (g->order == NULL)
		goto err;

	g->edges = calloc(esz, nkeys);
	if (g->edges == NULL)
		goto err;

	g->oedges = calloc(osz, nverts);
	if (g->oedges == NULL)
		goto err;

	g->nkeys = nkeys;
	g->nverts = nverts;
	g->flags |= ZEROED; // calloc

	return g;
err:
	save_errno = errno;
	rgph_free_graph(g);
	errno = save_errno;
	return NULL;
}

extern "C"
int
rgph_build_graph(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, unsigned long seed)
{
	const int r = graph_rank(g->flags);
	const size_t width = data_width(g->nverts, MIN_WIDTH_BUILD);

	if (!(g->flags & ZEROED)) {
		const size_t esz = edge_size(r, g->nverts, MIN_WIDTH_BUILD);
		const size_t osz = oedge_size(r, g->nverts, MIN_WIDTH_BUILD);
		memset(g->order, 0, esz * g->nkeys);
		memset(g->edges, 0, esz * g->nkeys);
		memset(g->oedges, 0, osz * g->nverts);
	}

	g->flags &= ~ZEROED;

#define SELECT(r, w) (8 * (r) + (w))
	switch (SELECT(r, width)) {
		case SELECT(2, 1):
			return build_graph<uint8_t,2>(g, keys, state, seed);
		case SELECT(3, 1):
			return build_graph<uint8_t,3>(g, keys, state, seed);
		case SELECT(2, 2):
			return build_graph<uint16_t,2>(g, keys, state, seed);
		case SELECT(3, 2):
			return build_graph<uint16_t,3>(g, keys, state, seed);
		case SELECT(2, 4):
			return build_graph<uint32_t,2>(g, keys, state, seed);
		case SELECT(3, 4):
			return build_graph<uint32_t,3>(g, keys, state, seed);
	    default:
		errno = EINVAL;
		return -1;
	}
#undef SELECT
}

extern "C"
size_t
rgph_count_keys(rgph_entry_iterator_t iter, void *state)
{
	entry_iterator keys(iter, state), end;
	size_t res = 0;

	for (; keys != end; ++keys)
		res++;
	return res;
}
