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

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

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

 /*
  * Generated graphs are always R-partite. This means that v0 is less
  * than (nverts / R), v1 starts from (nverts / R) and is less than
  * (2 * nverts / R). If R==3, v2 takes the remaining range.
  * All partitions in a generated graph have equal number of vertices,
  * that is, nverts is always a multiple of R.
  */
template<class T, int R>
struct edge {
	T verts[R]; // v0, v1 (and v2, if R==3).
};

/*
 * Data type for a valid oriented edge (v0, v1, v2), v1 < v2.
 * The first vertex v0 is implicit and is determined by an index
 * of the corresponding element in the oedges array.
 * When the algorithm starts, the degree and overts are set to zeroes.
 * Every time an edge is added or removed, the edge's verts values
 * are XORed with corresponding overts values.
 */
template<class T, int R>
struct oedge {
	T overts[R-1]; // XORed v1 (and v2, if R==3).
	T degree;      // Degree of v0.
	T edge;
};

// Assign initial value in bdz_assign().
template<class T, int R>
struct bdz_assigner
{
	size_t operator()(T, size_t i) const
	{

		return i;
	}
};

// Assign initial value in chm_assign().
template<class T, int R>
struct chm_assigner
{
	const T *index;

	T operator()(T e, size_t) const
	{

		return index[e];
	}
};

/*
 * Hash table of pointers to potentially duplicate keys can be build
 * without any additional hashing because v0 is a good hash for a table
 * with (nverts / R) entries.
 * Each entry is a pointer to (edge, keylen, key bytes) tuple.
 */
typedef size_t *duphash_entry_t;

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
operator==(const entry_iterator &a, const entry_iterator &b)
{

	return a.cur == b.cur;
}

inline bool
operator!=(const entry_iterator &a, const entry_iterator &b)
{

	return a.cur != b.cur;
}

template<class T, int R, class S, class H>
struct hash {
	typedef void (*func_t)(const void *, size_t, S, H *);
	func_t func;
	T hashes[4]; // Some hashes are x4.
	S seed;

	inline hash(func_t f, S s) : func(f), seed(s) {}

	inline const T *
	operator()(const void *key, size_t keylen) {
		bool_selector<(sizeof(T) == sizeof(H))> selector;
		return this->hashit(selector, key, keylen);
	}

	inline const T *
	hashit(bool_selector<true>, const void *key, size_t keylen) {
		func(key, keylen, seed, hashes);
		return hashes;
	}
	inline const T *
	hashit(bool_selector<false>, const void *key, size_t keylen) {
		H h[4]; // Some hashes are x4.
		func(key, keylen, seed, h);
		for (size_t i = 0; i < R; i++)
			hashes[i] = h[i];
		return hashes;
	}
};

template<class T, int R, class S, class H>
inline hash<T,R,S,H>
make_hash(void (*func)(const void *, size_t, S, H *), unsigned long seed)
{

	return hash<T,R,S,H>(func, seed);
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
remove_vertex(oedge<T,2> *oedges, T v0, T *order, size_t top)
{

	if (oedges[v0].degree == 1) {
		const T e = oedges[v0].edge;
		const T v1 = oedges[v0].overts[0];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0);
		order[--top] = e;
	}

	return top;
}

template<class T>
inline size_t
remove_vertex(oedge<T,3> *oedges, T v0, T *order, size_t top)
{

	if (oedges[v0].degree == 1) {
		const T e = oedges[v0].edge;
		const T v1 = oedges[v0].overts[0];
		const T v2 = oedges[v0].overts[1];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0, v2);
		remove_oedge(oedges, e, v2, v0, v1);
		order[--top] = e;
	}

	return top;
}

template<class Iter, class Hash, class T, int R>
bool
init_graph(Iter keys, Iter keys_end, Hash hash,
    edge<T,R> *edges, size_t nkeys, oedge<T,R> *oedges, size_t nverts,
    size_t *datalenmin, size_t *datalenmax, T *index)
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
		if (ent.datalen < *datalenmin)
			*datalenmin = ent.datalen;
		if (ent.datalen > *datalenmax)
			*datalenmax = ent.datalen;
		if (index != NULL)
			index[e] = ent.index < nkeys ? ent.index : e;
	}

	return e == nkeys;
}

template<class T, int R>
size_t
peel_graph(edge<T,R> *edges, size_t nkeys,
    oedge<T,R> *oedges, size_t nverts, T *order)
{
	size_t top = nkeys;

	for (T v0 = 0; v0 < nverts; ++v0)
		top = remove_vertex(oedges, v0, order, top);

	for (size_t i = nkeys; i > 0 && i > top; --i) {
		const edge<T,R> &e = edges[order[i-1]];
		for (size_t r = 0; r < R; ++r)
			top = remove_vertex(oedges, e.verts[r], order, top);
	}

	return top;
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
edge_size_impl(size_t width)
{

	switch (width) {
	case 1: return sizeof(edge<uint8_t,R>);
	case 2: return sizeof(edge<uint16_t,R>);
	case 4: return sizeof(edge<uint32_t,R>);
	default: return 0;
	}
}

inline size_t
edge_size(int rank, size_t width)
{

	switch (rank) {
	case 2: return edge_size_impl<2>(width);
	case 3: return edge_size_impl<3>(width);
	default: return 0;
	}
}

inline size_t
round_up(size_t n, size_t r)
{

	return n > -r ? 0 : (n + (r - 1)) / r * r;
}

inline size_t
round_up_pow2(size_t n)
{
	size_t r = 1;

	if ((n & (n - 1)) == 0)
		return n;

	while (n != 0) {
		n >>= 1;
		r <<= 1;
	}

	return r;
}

inline size_t
maxsize(size_t a, size_t b)
{

	return a > b ? a : b;
}

template<class T, int R>
inline size_t
duphash_size(size_t nverts)
{
	const size_t nelems = nverts / R;

	assert((nverts % R) == 0);

	if (nelems > SIZE_MAX / sizeof(duphash_entry_t))
		return 0;

	// Round up to T's size because the hash table is followed by
	// a peel index array. In practice, rounding doesn't make any
	// difference because T isn't bigger than duphash_entry_t.
	return round_up(nelems * sizeof(duphash_entry_t), sizeof(T));
}

/*
 * Memory allocated for oedges is shared with a hash table and a peel
 * index. Typically, oedges takes more space but it can take less
 * space for small T.
 * It's also shared with the assign functions but they need less
 * space: chm takes T[nverts] elements which is always smaller than
 * oedge<T,R>[nverts], bdz needs only nverts bytes.
 */
template<class T, int R>
inline size_t
oedges_size_impl(size_t nkeys, size_t nverts)
{
	assert(nverts > nkeys);

	const size_t osz = sizeof(oedge<T,R>);
	const size_t tsz = sizeof(T);
	// static_assert(osz > tsz);

	// Overflow check for nverts * osz and nverts * tsz:
	if (nverts > SIZE_MAX / osz)
		return 0;

	const size_t oedges_sz = nverts * osz;
	const size_t hash_sz = duphash_size<T,R>(nverts);
	if (hash_sz == 0)
		return 0;

	const size_t index_sz = nkeys * tsz;
	if (index_sz > SIZE_MAX - hash_sz)
		return 0;

	return maxsize(oedges_sz, hash_sz + index_sz);
}

template<int R>
inline size_t
oedges_size_impl(size_t width, size_t nkeys, size_t nverts)
{

	switch (width) {
	case 1: return oedges_size_impl<uint8_t,R>(nkeys, nverts);
	case 2: return oedges_size_impl<uint16_t,R>(nkeys, nverts);
	case 4: return oedges_size_impl<uint32_t,R>(nkeys, nverts);
	default: return 0;
	}
}

inline size_t
oedges_size(int rank, size_t width, size_t nkeys, size_t nverts)
{

	switch (rank) {
	case 2: return oedges_size_impl<2>(width, nkeys, nverts);
	case 3: return oedges_size_impl<3>(width, nkeys, nverts);
	default: return 0;
	}
}

inline int
graph_rank(unsigned int flags)
{

	return (flags & RGPH_RANK_MASK) == RGPH_RANK2 ? 2 : 3;
}

// The destination array is often called g in computer science literature.
template<class G, class T, int R, class A>
inline void
assign(const edge<T,R> *edges, const T *order, size_t nkeys,
    G *g, size_t nverts, G unassigned, A assigner)
{
	for (size_t v = 0; v < nverts; v++)
		g[v] = unassigned;

	for (size_t i = 0; i < nkeys; i++) {
		const T e = order[i];
		assert(e < nkeys);

		for (size_t j = 0; j < R; j++) {
			const T v = edges[e].verts[j];
			assert(v < nverts);

			if (g[v] != unassigned)
				continue;

			g[v] = assigner(e, j); // j for bdz and index[e] for chm
			assert(g[v] < unassigned);

			for (size_t k = 1; k < R; k++) {
				const T u = edges[e].verts[(j + k) % R];
				// Some compilers aren't smart enough to
				// use hints inside asserts in NDEBUG build.
				// Cache g[u] value to avoid reloading it
				// after writing to g[v]:
				const T gu = g[u];
				assert(u != v);

				// g[v] = (g[v] - g[u]) mod unassigned:
				if (g[v] < gu)
					g[v] += unassigned;
				g[v] -= gu;

				// Assign all traversed vertices:
				if (gu == unassigned)
					g[u] = 0;
			}

			assert(g[v] < unassigned);
		}
	}
}

} // namespace

struct rgph_graph {
	size_t nkeys;
	size_t nverts;
	size_t width; // sizeof(T)
	void *order;  // Output order of edges, points to T[nkeys] array.
	void *edges;  // Points to edge<T,R>[nkeys] array.
	void *oedges; // oedge<T,R>[nverts], can be reused for peel order index.
	void *index;  // Data index for RGPH_ALGO_CHM.
	size_t core_size; // R-core size.
	size_t datalenmin;
	size_t datalenmax;
	unsigned long seed;
	unsigned int flags;
};

enum {
	PUBLIC_FLAGS = 0x7fff,
	ZEROED   = 0x40000000, // The order, edges and oedges arrays are zeroed.
	BUILT    = 0x20000000, // Graph is built.
	PEELED   = 0x10000000, // Peel order index is built.
	ASSIGNED = 0x08000000  // Assignment step is done.
};

template<class T, int R>
static int
build_graph(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, unsigned long seed)
{
	typedef edge<T,R> edge_t;
	typedef oedge<T,R> oedge_t;

	T *order = (T *)g->order;
	T *index = (T *)g->index; // For RGPH_ALGO_CHM.
	edge_t *edges = (edge_t *)g->edges;
	oedge_t *oedges = (oedge_t *)g->oedges;

	if (!(g->flags & ZEROED)) {
		memset(order, 0, sizeof(T) * g->nkeys);
		memset(edges, 0, sizeof(edge_t) * g->nkeys);
		memset(oedges, 0, sizeof(oedge_t) * g->nverts);
	}

	g->width = sizeof(T);
	g->core_size = g->nkeys;
	g->datalenmin = SIZE_MAX;
	g->datalenmax = 0;
	g->seed = seed;
	g->flags &= PUBLIC_FLAGS; // Unset (ZEROED|BUILT|PEELED|ASSIGNED).

	entry_iterator keys_start(keys, state), keys_end;

	switch (g->flags & RGPH_HASH_MASK) {
	case RGPH_HASH_JENKINS2:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32x3_jenkins2_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax, index)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_MURMUR32:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32x4_murmur32_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax, index)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_MURMUR32S:
		if (R == 2) {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u16x2_murmur32s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		} else {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u8x4_murmur32s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		}
		break;
	case RGPH_HASH_XXH32S:
		if (R == 2) {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u16x2_xxh32s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		} else {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u8x4_xxh32s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		}
		break;
	case RGPH_HASH_XXH64S:
		if (R == 2) {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u32x2_xxh64s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		} else {
			if (!init_graph(keys_start, keys_end,
			    make_hash<T,R>(&rgph_u16x4_xxh64s_data, seed),
			    edges, g->nkeys, oedges, g->nverts,
			    &g->datalenmin, &g->datalenmax, index)) {
				return RGPH_NOKEY;
			}
		}
		break;
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}

	g->core_size = peel_graph(edges, g->nkeys, oedges, g->nverts, order);

	g->flags |= BUILT;
	return g->core_size == 0 ? RGPH_SUCCESS : RGPH_AGAIN;
}

template<class T, int R>
static const T *
build_peel_index(struct rgph_graph *g)
{
	const size_t hash_sz = duphash_size<T,R>(g->nverts);
	T *peel = (T *)((char *)g->oedges + hash_sz); // Reuse oedges.

	assert(hash_sz != 0);

	if (!(g->flags & PEELED)) {
		const T *order = (const T *)g->order;

		g->flags |= PEELED;
		g->flags &= ~ASSIGNED;
		memset(peel, 0, sizeof(T) * g->nkeys);

		for (size_t i = g->nkeys; i > g->core_size; i--) {
			assert(peel[order[i-1]] == 0);
			peel[order[i-1]] = g->nkeys - i + 1;
		}
	}

	return peel;
}

template<class T, int R>
static int
copy_edge(struct rgph_graph *g, size_t e, unsigned long *to, size_t *peel_order)
{
	typedef edge<T,R> edge_t;

	edge_t *edges = (edge_t *)g->edges;

	for (size_t r = 0; r < R; r++)
		to[r] = edges[e].verts[r];

	if (peel_order != NULL) {
		const T *peel = build_peel_index<T,R>(g);
		*peel_order = peel[e];
	}

	return RGPH_SUCCESS;
}

template<class T, int R>
static int
find_duplicates(struct rgph_graph *g,
    rgph_entry_iterator_t iter, void *state, size_t *dup)
{
	typedef edge<T,R> edge_t;

	assert((g->nverts % R) == 0);

	const size_t hash_sz = g->nverts / R;
	const size_t maxfill = g->nverts / 4; // Fill factor is 50% or 75%.
	duphash_entry_t *hash = (duphash_entry_t *)g->oedges; // Reuse oedges.

	for (size_t i = 0; i < hash_sz; i++)
		hash[i] = NULL;

	int res = RGPH_NOKEY;
	const T *peel = build_peel_index<T,R>(g);
	const edge_t *edges = (const edge_t *)g->edges;
	entry_iterator keys(iter, state), keys_end;

	size_t hashed = 0;
	for (size_t e = 0; e < g->nkeys; ++e, ++keys) {
		if (keys == keys_end) {
			res = RGPH_NOKEY;
			goto out;
		}

		if (peel[e] != 0)
			continue;

		if (hashed++ == maxfill) {
			res = RGPH_AGAIN;
			goto out;
		}

		const void *key = keys->key;
		const size_t keylen = keys->keylen;
		const T v0 = edges[e].verts[0];

		assert(v0 < hash_sz);

		// Linear probing with a wrap-around.
		size_t v = v0, vend = hash_sz;
		for (int i = 0; i < 2; i++) {
			for (; v < vend && hash[v] != NULL; v++) {
				if (keylen == hash[v][1] &&
				    memcmp(key, &hash[v][2], keylen) == 0) {
					res = RGPH_SUCCESS;
					dup[0] = hash[v][0];
					dup[1] = e;
					goto out;
				}
			}

			if (v < vend || i == 1)
				break;
			v = 0;
			vend = v0;
		}

		if (hash[v] != NULL) {
			res = RGPH_AGAIN; // Hash table is full.
			goto out;
		}

		if (keylen > SIZE_MAX - 2 * sizeof(hash[v][0])) {
			res = RGPH_NOMEM;
			goto out;
		}

		hash[v] =
		    (duphash_entry_t)malloc(2 * sizeof(hash[v][0]) + keylen);
		if (hash[v] == NULL) {
			res = RGPH_NOMEM;
			goto out;
		}

		hash[v][0] = e;
		hash[v][1] = keylen;
		memcpy(&hash[v][2], key, keylen);
	}
out:
	for (size_t i = 0; i < hash_sz; i++) {
		if (hash[i] != NULL)
			free(hash[i]);
	}

	if (res == RGPH_NOMEM)
		errno = ENOMEM;

	return res;
}

template<class T, int R>
static int
assign_bdz(struct rgph_graph *g)
{
	typedef edge<T,R> edge_t;

	const T *order = (const T *)g->order;
	const edge_t *edges = (const edge_t *)g->edges;
	uint8_t *assigned = (uint8_t *)g->oedges; // Reuse oedges.
	const uint8_t unassigned = R;
	const bdz_assigner<T,R> assigner;

	assert(g->core_size == 0);

	g->flags |= ASSIGNED;
	g->flags &= ~PEELED;

	assign(edges, order, g->nkeys,
	    assigned, g->nverts, unassigned, assigner);

	return RGPH_SUCCESS;
}

template<class T, int R>
static int
assign_chm(struct rgph_graph *g)
{
	typedef edge<T,R> edge_t;

	const T *order = (const T *)g->order;
	const T *index = (const T *)g->index;
	const edge_t *edges = (const edge_t *)g->edges;
	T *assigned = (T *)g->oedges; // Reuse oedges.
	const T unassigned = g->nkeys;
	const chm_assigner<T,R> assigner = { index };

	assert(index != NULL);
	assert(g->core_size == 0);

	g->flags |= ASSIGNED;
	g->flags &= ~PEELED;

	assign(edges, order, g->nkeys,
	    assigned, g->nverts, unassigned, assigner);

	return RGPH_SUCCESS;
}

template<class T, int R>
static int
assign(struct rgph_graph *g)
{

	switch (g->flags & RGPH_ALGO_MASK) {
	case RGPH_ALGO_BDZ:
		return assign_bdz<T,R>(g);
	case RGPH_ALGO_CHM:
		return assign_chm<T,R>(g);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
void
rgph_free_graph(struct rgph_graph *g)
{

	free(g->index);
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
	size_t maxkeys, nverts, width, esz, osz;
	int save_errno;
	int r;

	if (flags & ~PUBLIC_FLAGS) {
		errno = EINVAL;
		return NULL;
	}

	if ((flags & RGPH_RANK_MASK) == RGPH_RANK_DEFAULT)
		flags |= RGPH_RANK3;

	if ((flags & RGPH_ALGO_MASK) == RGPH_ALGO_DEFAULT)
		flags |= RGPH_ALGO_CHM;

	// XXX Decide on default.
	if ((flags & RGPH_HASH_MASK) == RGPH_HASH_DEFAULT)
		flags |= RGPH_HASH_JENKINS2;

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

	if (flags & RGPH_ROUND_POW2) {
		nverts = round_up_pow2(nverts / r) * r;
		if (nverts == 0) {
			errno = ERANGE;
			return NULL;
		}
	}

	assert(nverts > nkeys);

	width = data_width(nverts, MIN_WIDTH_BUILD);
	esz = edge_size(r, width);
	if (esz == 0) {
		errno = EINVAL;
		return NULL;
	}

	osz = oedges_size(r, width, nkeys, nverts);
	if (osz == 0) {
		errno = ENOMEM;
		return NULL;
	}

	switch (flags & RGPH_HASH_MASK) {
	case RGPH_HASH_JENKINS2:
	case RGPH_HASH_MURMUR32:
		break;
	case RGPH_HASH_XXH32S:
	case RGPH_HASH_MURMUR32S:
		if (nverts > r * (r == 3 ? 255u : 65535u)) {
			errno = ERANGE;
			return NULL;
		}
		break;
	case RGPH_HASH_XXH64S:
		if (r == 3 && nverts > r * 65535u) {
			errno = ERANGE;
			return NULL;
		}
		break;
	default:
		errno = EINVAL;
		return NULL;
	}

	g = (struct rgph_graph *)calloc(sizeof(*g), 1);
	if (g == NULL)
		return NULL;

	g->nkeys  = 0;
	g->nverts = 0;
	g->order  = NULL;
	g->edges  = NULL;
	g->oedges = NULL;
	g->index  = NULL;
	g->seed   = 0;
	g->flags  = flags;

	g->order = calloc(width, nkeys);
	if (g->order == NULL)
		goto err;

	g->edges = calloc(esz, nkeys);
	if (g->edges == NULL)
		goto err;

	g->oedges = calloc(osz, 1);
	if (g->oedges == NULL)
		goto err;

	if (flags & RGPH_ALGO_CHM) {
		g->index = malloc(width * nkeys);
		if (g->order == NULL)
			goto err;
	}

	g->nkeys = nkeys;
	g->nverts = nverts;
	g->core_size = nkeys;
	g->datalenmin = SIZE_MAX;
	g->datalenmax = 0;
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
rgph_flags(struct rgph_graph *g)
{

	return g->flags & PUBLIC_FLAGS;
}

extern "C"
int
rgph_rank(struct rgph_graph *g)
{

	return graph_rank(g->flags);
}

extern "C"
int
rgph_is_bdz(struct rgph_graph *g)
{

	return (g->flags & RGPH_ALGO_BDZ) != 0;
}

extern "C"
int rgph_is_chm(struct rgph_graph *g)
{

	return (g->flags & RGPH_ALGO_CHM) != 0;
}

extern "C"
size_t
rgph_entries(struct rgph_graph *g)
{

	return g->nkeys;
}

extern "C"
size_t
rgph_vertices(struct rgph_graph *g)
{

	return g->nverts;
}

extern "C"
size_t
rgph_core_size(struct rgph_graph *g)
{

	return g->core_size;
}

extern "C"
unsigned long
rgph_seed(struct rgph_graph *g)
{

	return g->seed;
}

extern "C"
int
rgph_is_built(struct rgph_graph *g)
{

	return (g->flags & BUILT) != 0;
}

extern "C"
int
rgph_is_assigned(struct rgph_graph *g)
{
	int res = (g->flags & ASSIGNED) != 0;

	assert(res != ((g->flags & PEELED) != 0));
	return res;
}

extern "C"
int
rgph_build_graph(struct rgph_graph *g,
    unsigned long seed, rgph_entry_iterator_t keys, void *state)
{
	const int r = graph_rank(g->flags);
	const size_t width = data_width(g->nverts, MIN_WIDTH_BUILD);

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
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
#undef SELECT
}

extern "C"
int
rgph_copy_edge(struct rgph_graph *g, size_t edge,
    unsigned long *to, size_t *peel_order)
{
	const int r = graph_rank(g->flags);
	const size_t width = data_width(g->nverts, MIN_WIDTH_BUILD);

	if (!(g->flags & BUILT))
		return RGPH_INVAL;

	if (edge >= g->nkeys)
		return RGPH_RANGE;

#define SELECT(r, w) (8 * (r) + (w))
	switch (SELECT(r, width)) {
	case SELECT(2, 1):
		return copy_edge<uint8_t,2>(g, edge, to, peel_order);
	case SELECT(3, 1):
		return copy_edge<uint8_t,3>(g, edge, to, peel_order);
	case SELECT(2, 2):
		return copy_edge<uint16_t,2>(g, edge, to, peel_order);
	case SELECT(3, 2):
		return copy_edge<uint16_t,3>(g, edge, to, peel_order);
	case SELECT(2, 4):
		return copy_edge<uint32_t,2>(g, edge, to, peel_order);
	case SELECT(3, 4):
		return copy_edge<uint32_t,3>(g, edge, to, peel_order);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
#undef SELECT
}

extern "C"
int
rgph_find_duplicates(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, size_t *dup)
{
	const int r = graph_rank(g->flags);
	const size_t width = data_width(g->nverts, MIN_WIDTH_BUILD);

	if (!(g->flags & BUILT))
		return RGPH_INVAL;

#define SELECT(r, w) (8 * (r) + (w))
	switch (SELECT(r, width)) {
	case SELECT(2, 1):
		return find_duplicates<uint8_t,2>(g, keys, state, dup);
	case SELECT(3, 1):
		return find_duplicates<uint8_t,3>(g, keys, state, dup);
	case SELECT(2, 2):
		return find_duplicates<uint16_t,2>(g, keys, state, dup);
	case SELECT(3, 2):
		return find_duplicates<uint16_t,3>(g, keys, state, dup);
	case SELECT(2, 4):
		return find_duplicates<uint32_t,2>(g, keys, state, dup);
	case SELECT(3, 4):
		return find_duplicates<uint32_t,3>(g, keys, state, dup);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
#undef SELECT
}

extern "C"
int
rgph_assign(struct rgph_graph *g)
{
	const int r = graph_rank(g->flags);
	const size_t width = data_width(g->nverts, MIN_WIDTH_BUILD);

	if (!(g->flags & BUILT))
		return RGPH_INVAL;
	else if (g->core_size != 0)
		return RGPH_AGAIN;

#define SELECT(r, w) (8 * (r) + (w))
	switch (SELECT(r, width)) {
	case SELECT(2, 1):
		return assign<uint8_t,2>(g);
	case SELECT(3, 1):
		return assign<uint8_t,3>(g);
	case SELECT(2, 2):
		return assign<uint16_t,2>(g);
	case SELECT(3, 2):
		return assign<uint16_t,3>(g);
	case SELECT(2, 4):
		return assign<uint32_t,2>(g);
	case SELECT(3, 4):
		return assign<uint32_t,3>(g);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
#undef SELECT
}

extern "C"
const void *
rgph_assignments(struct rgph_graph *g, size_t *width)
{
	const unsigned int flags = g->flags;
	const void *assigned = g->oedges; // Reused.

	if (width != NULL)
		*width = (flags & RGPH_ALGO_BDZ) ? 1 : g->width;

	if (flags & ASSIGNED)
		return assigned;
	else
		return NULL;
}

extern "C"
int rgph_copy_assignment(struct rgph_graph *g, size_t n, unsigned int *to)
{
	const unsigned int flags = g->flags;
	const size_t width = (flags & RGPH_ALGO_BDZ) ? 1 : g->width;
	const void *assigned = g->oedges; // Reused.

	if (!(flags & ASSIGNED))
		return RGPH_INVAL;

	if (n >= g->nverts)
		return RGPH_RANGE;

	switch (width) {
	case 1:
		*to = ((const uint8_t *)assigned)[n];
		return RGPH_SUCCESS;
	case 2:
		*to = ((const uint16_t *)assigned)[n];
		return RGPH_SUCCESS;
	case 4:
		*to = ((const uint32_t *)assigned)[n];
		return RGPH_SUCCESS;
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
size_t
rgph_count_keys(rgph_entry_iterator_t iter, void *state)
{
	entry_iterator keys(iter, state), keys_end;
	size_t res = 0;

	for (; keys != keys_end; ++keys)
		res++;
	return res;
}
