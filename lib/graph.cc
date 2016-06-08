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
#include "rgph_bitops.h"
#include "rgph_fastdiv.h"
#include "rgph_graph.h"
#include "rgph_hash.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffffu
#endif

// Max nkeys values.
#define MAX_NKEYS_R2_VEC 0x78787877u // Vector hashes.
#define MAX_NKEYS_R2_S64 0x78787877u // Scalar 64 hashes.
#define MAX_NKEYS_R2_S32 0x00007878u // Scalar 32 hashes.
#define MAX_NKEYS_R3_VEC 0xcccccccau // Vector hashes.
#define MAX_NKEYS_R3_S64 0x00266666u // Scalar 64 hashes.
#define MAX_NKEYS_R3_S32 0x000004ccu // Scalar 32 hashes.

// Max values for fastdiv divisors.
#define MAX_FASTDIV_R2_VEC 0x7fff8000u // Vector hashes.
#define MAX_FASTDIV_R2_S64 0x7fff8000u // Scalar 64 hashes.
#define MAX_FASTDIV_R2_S32 0x00007fffu // Scalar 32 hashes.
#define MAX_FASTDIV_R3_VEC 0x55555555u // Vector hashes.
#define MAX_FASTDIV_R3_S64 0x000fffffu // Scalar 64 hashes.
#define MAX_FASTDIV_R3_S32 0x000001ffu // Scalar 32 hashes.

namespace {

typedef uint32_t vert_t; // vertex or key
typedef uint32_t index_t;

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
struct bdz_assigner
{
	size_t operator()(vert_t, size_t i) const
	{

		return i;
	}
};

// Assign initial value in chm_assign().
struct chm_assigner
{
	const index_t *index;

	vert_t operator()(vert_t e, size_t) const
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

// Scalar hash returns a scalar value of type H.
template<class T, int R, class H>
struct scalar_hash {
	typedef H (*func_t)(const void *, size_t, uint32_t);

	const func_t func;
	const unsigned long seed;
	T hashes[3];

	inline scalar_hash(func_t f, unsigned long seed)
		: func(f)
		, seed(seed)
	{}

	inline const T *
	operator()(const void *key, size_t keylen) {
		const size_t nbits = sizeof(H) * CHAR_BIT / R;
		const H mask = (H(1) << nbits) - 1;

		H h = func(key, keylen, seed);
		for (size_t i = 0; i < R - 1; i++)
			hashes[i] = (h >> i * nbits) & mask;
		hashes[R - 1] = h >> (sizeof(H) * CHAR_BIT - nbits);
		return hashes;
	}
};

// Vector hash initialises an array of hash values (x3 or x4).
template<class T, int R, class H>
struct vector_hash {
	typedef void (*func_t)(const void *, size_t, uint32_t, H *);

	const func_t func;
	const unsigned long seed;
	T hashes[4]; // Some hashes are x4.

	inline vector_hash(func_t f, unsigned long seed)
		: func(f)
		, seed(seed)
	{}

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

template<class T, int R, class H>
inline scalar_hash<T,R,H>
make_hash(H (*func)(const void *, size_t, uint32_t), unsigned long seed)
{

	return scalar_hash<T,R,H>(func, seed);
}

template<class T, int R, class H>
inline vector_hash<T,R,H>
make_hash(void (*func)(const void *, size_t, uint32_t, H *), unsigned long seed)
{

	return vector_hash<T,R,H>(func, seed);
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

// fast_divide32(3) from NetBSD.
inline uint32_t
fastdiv(uint32_t val, uint64_t mul, uint8_t s1, uint8_t s2)
{
	const uint32_t hi = (val * mul) >> 32;

	return (hi + ((val - hi) >> s1)) >> s2;
}

// fast_remainder32(3) from NetBSD.
inline uint32_t
fastrem(uint32_t val, uint32_t div, uint64_t mul, uint8_t s1, uint8_t s2)
{

	return val - div * fastdiv(val, mul, s1, s2);
}

template<class Iter, class Hash, class T, int R>
bool
init_graph(Iter keys, Iter keys_end, Hash hash,
    edge<T,R> *edges, size_t nkeys, oedge<T,R> *oedges, size_t nverts,
    size_t *datalenmin, size_t *datalenmax,
    T *index, size_t *indexmin, size_t *indexmax)
{
	// partsz is a partition size of an R-partite R-graph.
	const T partsz = nverts / R;
	// Fast division by partsz.
	uint32_t mul;
	uint8_t s1, s2;

	assert(partsz > 1 && (nverts % R) == 0);

	if (index == NULL) {
		// bdz
		*indexmin = 0;
		*indexmax = R - 1;
	}

	rgph_fastdiv_prepare(partsz, &mul, &s1, &s2, 0, NULL);

	T e = 0;
	for (; e < nkeys && keys != keys_end; ++e, ++keys) {
		const rgph_entry &ent = *keys;
		const T *verts = hash(ent.key, ent.keylen);
		for (T r = 0; r < R; ++r)
			edges[e].verts[r] = r * partsz +
			    fastrem(verts[r], partsz, mul, s1, s2);
		add_edge(oedges, e, edges[e].verts);
		if (ent.datalen < *datalenmin)
			*datalenmin = ent.datalen;
		if (ent.datalen > *datalenmax)
			*datalenmax = ent.datalen;
		if (index != NULL) {
			// chm
			const size_t i = ent.index == SIZE_MAX ? e : ent.index;
			index[e] = i;
			if (i < *indexmin)
				*indexmin = i;
			if (i > *indexmax)
				*indexmax = i;
		}
	}

	return e == nkeys;
}

void
init_index(index_t *index, size_t nkeys)
{
	size_t i;

	for (i = 0; i < nkeys; i++)
		index[i] = i;
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
edge_size(int rank)
{

	switch (rank) {
	case 2: return sizeof(edge<vert_t,2>);
	case 3: return sizeof(edge<vert_t,3>);
	default: return 0;
	}
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

inline size_t
oedges_size(int rank, size_t nkeys, size_t nverts)
{

	switch (rank) {
	case 2: return oedges_size_impl<vert_t,2>(nkeys, nverts);
	case 3: return oedges_size_impl<vert_t,3>(nkeys, nverts);
	default: return 0;
	}
}

size_t
hash_bits(unsigned int flags)
{

	switch (flags & RGPH_HASH_MASK) {
	case RGPH_HASH_JENKINS2V:
		return 96;
	case RGPH_HASH_MURMUR32V:
		return 128;
	case RGPH_HASH_MURMUR32S:
		return 32;
	case RGPH_HASH_XXH32S:
		return 32;
	case RGPH_HASH_XXH64S:
		return 64;
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return 0;
	}
}

inline int
graph_rank(unsigned int flags)
{

	return (flags & RGPH_RANK_MASK) == RGPH_RANK2 ? 2 : 3;
}

// See ../examples/calc-constants.c.
inline size_t
graph_max_keys(unsigned int flags)
{
	const int rank = graph_rank(flags);
	const size_t nbits = hash_bits(flags);

	if (nbits >= 96)
		return (rank == 2) ? MAX_NKEYS_R2_VEC : MAX_NKEYS_R3_VEC;
	else if (nbits == 64)
		return (rank == 2) ? MAX_NKEYS_R2_S64 : MAX_NKEYS_R3_S64;
	else if (nbits == 32)
		return (rank == 2) ? MAX_NKEYS_R2_S32 : MAX_NKEYS_R3_S32;
	else {
		assert(0 && "rgph_alloc_graph() should have caught it");
		return 0;
	}
}

inline size_t
graph_max_fastdiv(unsigned int flags)
{
	const int rank = graph_rank(flags);
	const size_t nbits = hash_bits(flags);

	if (nbits >= 96)
		return (rank == 2) ? MAX_FASTDIV_R2_VEC : MAX_FASTDIV_R3_VEC;
	else if (nbits == 64)
		return (rank == 2) ? MAX_FASTDIV_R2_S64 : MAX_FASTDIV_R3_S64;
	else if (nbits == 32)
		return (rank == 2) ? MAX_FASTDIV_R2_S32 : MAX_FASTDIV_R3_S32;
	else {
		assert(0 && "rgph_alloc_graph() should have caught it");
		return 0;
	}
}

inline size_t
graph_nverts(int *flags, size_t nkeys)
{
	const int r = graph_rank(*flags);
	const bool full_range = (hash_bits(*flags) >= r * 32u);
	const size_t div_nbits = sizeof(vert_t) * CHAR_BIT - !full_range;
	const size_t max_nkeys = graph_max_keys(*flags);
	const size_t max_fastdiv = graph_max_fastdiv(*flags);

	if (nkeys == 0 || nkeys > max_nkeys)
		return 0;

	size_t nverts = (r == 2) ? 2 * nkeys + (nkeys + 7) / 8
	                         : 1 * nkeys + (nkeys + 3) / 4;
	nverts = round_up(nverts, r);
	if (nverts < 24)
		nverts = 24;

	assert((nverts % r) == 0);

	if ((*flags & RGPH_DIV_MASK) == RGPH_DIV_DEFAULT)
		return nverts;

	const size_t pow2_div = round_up_pow2(nverts / r);

	// Max divisor of a full range hash isn't a power of two.
	if (pow2_div > UINT32_MAX / r)
		*flags &= ~RGPH_DIV_POW2;

	if (nverts / r > max_fastdiv)
		*flags &= ~RGPH_DIV_FAST;

	if (!(*flags & RGPH_DIV_POW2) && !(*flags & RGPH_DIV_FAST))
		return 0;
	if ((*flags & RGPH_DIV_POW2) && !(*flags & RGPH_DIV_FAST))
		return pow2_div * r;

	const size_t max_div =
	    (*flags & RGPH_DIV_POW2) ? pow2_div : max_fastdiv;

	assert(nverts / r > 1 && nverts / r <= max_div);

	for (size_t div = nverts / r; div <= max_div; div++) {
		uint32_t mul;
		uint8_t s1, s2;
		int inc;

		// rgph_fastdiv_prepare() doesn't work for powers of 2.
		if (is_pow2(div))
			continue;

		rgph_fastdiv_prepare(div, &mul, &s1, &s2, div_nbits, &inc);
		if (s1 == 0 && inc == 0) {
			*flags &= ~RGPH_DIV_MASK;
			*flags |= RGPH_DIV_FAST;
			return div * r;
		}
	}

	assert(is_pow2(max_div));

	*flags &= ~RGPH_DIV_MASK;
	*flags |=  RGPH_DIV_POW2;
	return max_div * r;
}

// The destination array is often called g in computer science literature.
template<class G, class T, int R, class A>
inline void
assign(const edge<T,R> *edges, const T *order, size_t nkeys,
    A assigner, G *g, size_t nverts, size_t min, size_t max)
{
	const G unassigned = max + 1;
	assert(unassigned > max);

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

				// g[v] = min + (g[v] - g[u]) mod index_range:
				if (g[v] < gu)
					g[v] += unassigned - min;
				g[v] -= gu - min;

				// Assign all traversed vertices:
				if (gu == unassigned)
					g[u] = min;
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
	size_t indexmin;
	size_t indexmax;
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
	g->indexmin = SIZE_MAX;
	g->indexmax = 0;
	g->seed = seed;
	g->flags &= PUBLIC_FLAGS; // Unset (ZEROED|BUILT|PEELED|ASSIGNED).

	entry_iterator keys_start(keys, state), keys_end;

	switch (g->flags & RGPH_HASH_MASK) {
	case RGPH_HASH_JENKINS2V:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32x3_jenkins2v_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax,
		    index, &g->indexmin, &g->indexmax)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_MURMUR32V:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32x4_murmur32v_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax,
		    index, &g->indexmin, &g->indexmax)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_MURMUR32S:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32_murmur32s_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax,
		    index, &g->indexmin, &g->indexmax)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_XXH32S:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u32_xxh32s_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax,
		    index, &g->indexmin, &g->indexmax)) {
			return RGPH_NOKEY;
		}
		break;
	case RGPH_HASH_XXH64S:
		if (!init_graph(keys_start, keys_end,
		    make_hash<T,R>(&rgph_u64_xxh64s_data, seed),
		    edges, g->nkeys, oedges, g->nverts,
		    &g->datalenmin, &g->datalenmax,
		    index, &g->indexmin, &g->indexmax)) {
			return RGPH_NOKEY;
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
	const bdz_assigner assigner;

	assert(g->core_size == 0);

	g->flags |= ASSIGNED;
	g->flags &= ~PEELED;

	assign(edges, order, g->nkeys, assigner,
	    assigned, g->nverts, 0, R - 1);

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
	const T unassigned = g->indexmax + 1;
	const chm_assigner assigner = { index };

	// Check an overflow in g->indexmax + 1:
	if (unassigned < g->indexmax)
		return RGPH_RANGE;

	assert(index != NULL);
	assert(g->core_size == 0);

	g->flags |= ASSIGNED;
	g->flags &= ~PEELED;

	assign(edges, order, g->nkeys, assigner,
	    assigned, g->nverts, g->indexmin, g->indexmax);

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
	size_t nverts, esz, osz;
	int save_errno;
	int r;

	if (flags & ~PUBLIC_FLAGS) {
		errno = EINVAL;
		return NULL;
	}

	// Fail if both RGPH_ALGO_CHM and RGPH_ALGO_BDZ are passed.
	if ((flags & RGPH_ALGO_MASK) == RGPH_ALGO_MASK) {
		errno = EINVAL;
		return NULL;
	}

	// Don't accept bad hash flags.
	if ((flags & RGPH_HASH_MASK) > RGPH_HASH_LAST) {
		errno = EINVAL;
		return NULL;
	}

	if ((flags & RGPH_RANK_MASK) == RGPH_RANK_DEFAULT)
		flags |= RGPH_RANK3;

	if ((flags & RGPH_ALGO_MASK) == RGPH_ALGO_DEFAULT)
		flags |= RGPH_ALGO_CHM;

	if ((flags & RGPH_HASH_MASK) == RGPH_HASH_DEFAULT)
		flags |= RGPH_HASH_JENKINS2V;

	nverts = graph_nverts(&flags, nkeys);
	if (nverts == 0) {
		errno = ERANGE;
		return NULL;
	}

	assert(nverts > nkeys);

	r = graph_rank(flags);
	esz = edge_size(r);
	if (esz == 0) {
		errno = EINVAL;
		return NULL;
	}

	osz = oedges_size(r, nkeys, nverts);
	if (osz == 0) {
		errno = ENOMEM;
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

	g->order = calloc(sizeof(vert_t), nkeys);
	if (g->order == NULL)
		goto err;

	g->edges = calloc(esz, nkeys);
	if (g->edges == NULL)
		goto err;

	g->oedges = calloc(osz, 1);
	if (g->oedges == NULL)
		goto err;

	if (flags & RGPH_ALGO_CHM) {
		g->index = malloc(sizeof(index_t) * nkeys);
		if (g->index == NULL)
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
uint32_t
rgph_unassigned(struct rgph_graph *g)
{
	const unsigned int bdz = g->flags & RGPH_ALGO_BDZ;

	return bdz ? graph_rank(g->flags) : g->indexmax + 1;
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
rgph_datalen_min(struct rgph_graph *g)
{

	return g->datalenmin;
}

extern "C"
size_t
rgph_datalen_max(struct rgph_graph *g)
{

	return g->datalenmax;
}

extern "C"
size_t
rgph_index_min(struct rgph_graph *g)
{

	return g->indexmin;
}

extern "C"
size_t
rgph_index_max(struct rgph_graph *g)
{

	return g->indexmax;
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
size_t
rgph_hash_bits(struct rgph_graph *g)
{

	return hash_bits(g->flags);
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

	switch (graph_rank(g->flags)) {
	case 2:
		return build_graph<vert_t,2>(g, keys, state, seed);
	case 3:
		return build_graph<vert_t,3>(g, keys, state, seed);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
int
rgph_copy_edge(struct rgph_graph *g, size_t edge,
    unsigned long *to, size_t *peel_order)
{

	if (!(g->flags & BUILT))
		return RGPH_INVAL;

	if (edge >= g->nkeys)
		return RGPH_RANGE;

	switch (graph_rank(g->flags)) {
	case 2:
		return copy_edge<vert_t,2>(g, edge, to, peel_order);
	case 3:
		return copy_edge<vert_t,3>(g, edge, to, peel_order);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
int
rgph_find_duplicates(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, size_t *dup)
{

	if (!(g->flags & BUILT))
		return RGPH_INVAL;

	switch (graph_rank(g->flags)) {
	case 2:
		return find_duplicates<vert_t,2>(g, keys, state, dup);
	case 3:
		return find_duplicates<vert_t,3>(g, keys, state, dup);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
int
rgph_assign(struct rgph_graph *g, int algo)
{

	if (!(g->flags & BUILT))
		return RGPH_INVAL;
	else if (g->core_size != 0)
		return RGPH_AGAIN;
	else if (algo & ~RGPH_ALGO_MASK)
		return RGPH_INVAL;

	if (algo != 0) {
		if (algo != RGPH_ALGO_BDZ && algo != RGPH_ALGO_CHM)
			return RGPH_INVAL;

		if (algo == RGPH_ALGO_CHM && g->index == NULL) {
			g->index = malloc(g->width * g->nkeys);
			if (g->index == NULL)
				return RGPH_NOMEM;
			g->indexmin = 0;
			g->indexmax = g->nkeys - 1;
			init_index((index_t *)g->index, g->nkeys);
		}

		g->flags &= ~RGPH_ALGO_MASK;
		g->flags |= algo;
	}

	switch (graph_rank(g->flags)) {
	case 2:
		return assign<vert_t,2>(g);
	case 3:
		return assign<vert_t,3>(g);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
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
