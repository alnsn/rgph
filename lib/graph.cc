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

/*
 * The algorithm below is based on paper
 * Cache-Oblivious Peeling of Random Hypergraphs by Djamal Belazzougui,
 * Paolo Boldi, Giuseppe Ottaviano, Rossano Venturini, and Sebastiano
 * Vigna.
 * http://zola.di.unipi.it/rossano/wp-content/papercite-data/pdf/dcc14.pdf
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

// Max chm index values.
#define INDEX_MAX UINT32_MAX
#define BIG_INDEX_MAX UINT64_MAX

// Max nkeys values.
#define MAX_NKEYS_R2_VEC 0x78787877u // Vector hashes.
#define MAX_NKEYS_R2_S64 0x78787877u // Scalar 64 hashes.
#define MAX_NKEYS_R2_S32 0x00007878u // Scalar 32 hashes.
#define MAX_NKEYS_R3_VEC 0xcccccccau // Vector hashes.
#define MAX_NKEYS_R3_S64 0x00266666u // Scalar 64 hashes.
#define MAX_NKEYS_R3_S32 0x000004ccu // Scalar 32 hashes.

namespace {

enum {
	ZEROED   = 0x40000000, // The order, edges and oedges arrays are zeroed.
	BUILT    = 0x20000000, // Graph is built.
	PEELED   = 0x10000000, // Peel order index is built.
	ASSIGNED = 0x08000000, // Assignment step is done.
	PUBLIC_FLAGS = 0xffff
};

typedef uint32_t vert_t;         // Vertex or key; V in templates.
typedef uint32_t index_t;        // Chm index; X in templates.
typedef uint64_t big_index_t;    // Switch to big index if index_t is too small.
typedef uint8_t nullptr_index_t; // For internal use by init_graph().

/*
 * Generated graphs are always R-partite. This means that v0 is less
 * than (nverts / R), v1 starts from (nverts / R) and is less than
 * (2 * nverts / R). If R==3, v2 takes the remaining range.
 * All partitions in a generated graph have equal number of vertices,
 * that is, nverts is always a multiple of R.
 */
template<class V, int R>
struct edge {
	V verts[R]; // v0, v1 (and v2, if R==3).
};

/*
 * Data type for a valid oriented edge (v0, v1, v2), v1 < v2.
 * The first vertex v0 is implicit and is determined by an index
 * of the corresponding element in the oedges array.
 * When the algorithm starts, the degree and overts are set to zeroes.
 * Every time an edge is added or removed, the edge's verts values
 * are XORed with corresponding overts values.
 */
template<class V, int R>
struct oedge {
	V overts[R-1]; // XORed v1 (and v2, if R==3).
	V degree;      // Degree of v0.
	V edge;
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
	struct rgph_entry const *cur;

	inline entry_iterator();
	inline entry_iterator(rgph_entry_iterator_t i, void *s);

	inline void operator++();
	inline struct rgph_entry const *operator->() const;
	inline struct rgph_entry const &operator*() const;
};

// Good enough to detect past-the-end.
inline bool operator==(entry_iterator const &, entry_iterator const &);
inline bool operator!=(entry_iterator const &, entry_iterator const &);

// Vector hash initialises an array of R hash values.
template<class V, int R, class H>
struct vector_hash {
	static bool constexpr zerocopy = sizeof(H) == sizeof(V);

	typedef void (*func_t)(void const *, size_t, uint32_t, H *);

	func_t const func;
	unsigned long const seed;
	mutable V hashes[zerocopy ? 4 : R]; // Some hashes are x4.

	inline vector_hash(func_t f, unsigned long seed);

	inline V const *operator()(void const *, size_t) const;

	inline V const *impl(void const *, size_t, V (&)[4]) const;
	inline V const *impl(void const *, size_t, V (&)[R]) const;
};

// Scalar hash splits a scalar hash of type H into R hashes of type V.
template<class V, int R, class H>
struct scalar_hash {
	typedef H (*func_t)(void const *, size_t, uint32_t);

	func_t const func;
	unsigned long const seed;
	mutable V hashes[R];

	inline scalar_hash(func_t f, unsigned long seed);

	inline V const *operator()(void const *, size_t) const;
};

// Partition a graph using fast_remainder32(3) from NetBSD.
struct fastrem_partition {
	vert_t  partsz; // Partition size of an R-partite R-graph.
	vert_t  mul;
	uint8_t s2;

	inline fastrem_partition(size_t, size_t);

	inline uint32_t operator()(vert_t const *, size_t) const;
};

// http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
struct lemire_partition {
	vert_t  partsz; // Partition size of an R-partite R-graph.
	uint8_t shift;

	inline lemire_partition(size_t, size_t, size_t);

	inline uint32_t operator()(vert_t const *, size_t) const;
};

// Assign initial value in bdz_assign().
struct bdz_assigner {
	inline size_t operator()(vert_t, size_t) const;
};

// Assign initial value in chm_assign().
template<class X>
struct chm_assigner {
	X const *index;

	inline X operator()(vert_t, size_t) const;
};

} // anon namespace

struct rgph_graph {
	size_t nkeys;
	size_t nverts;
	void *order;  // Output order of edges, points to V[nkeys] array.
	void *edges;  // Points to edge<V,R>[nkeys] array.
	union {
		void *oedges;             // oedge<V,R>[nverts] is shared
		duphash_entry_t *duphash; // with a hash table for duplicates
		void    *chm_assignments; // and with chm/bdz assignments.
		uint8_t *bdz_assignments;
	} shared;
	void *index; // Data index for chm algorithm.
	int *assigned; // Use bitset when "unassigned" value isn't available.
	size_t core_size; // R-core size.
	size_t datalenmin;
	size_t datalenmax;
	big_index_t indexmin;
	big_index_t indexmax;
	unsigned long seed;
	unsigned int flags;
};

namespace {

template<class V>
inline void
add_remove_oedge(oedge<V,2> *oedges, int delta, V e, V v0, V v1)
{

	oedges[v0].overts[0] ^= v1;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class V>
inline void
remove_edge(oedge<V,2> *oedges, V e, V v0, V v1)
{

	return add_remove_oedge(oedges, -1, e, v0, v1);
}

template<class V>
inline void
add_remove_oedge(oedge<V,3> *oedges, int delta, V e, V v0, V v1, V v2)
{

	oedges[v0].overts[v1 < v2 ? 0 : 1] ^= v1;
	oedges[v0].overts[v1 < v2 ? 1 : 0] ^= v2;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class V>
inline void
remove_oedge(oedge<V,2> *oedges, V e, V v0, V v1)
{

	return add_remove_oedge(oedges, -1, e, v0, v1);
}

template<class V>
inline void
remove_oedge(oedge<V,3> *oedges, V e, V v0, V v1, V v2)
{

	return add_remove_oedge(oedges, -1, e, v0, v1, v2);
}

template<class V>
inline void
add_edge(oedge<V,2> *oedges, V e, V const *verts)
{

	add_remove_oedge(oedges, 1, e, verts[0], verts[1]);
	add_remove_oedge(oedges, 1, e, verts[1], verts[0]);
}

template<class V>
inline void
add_edge(oedge<V,3> *oedges, V e, V const *verts)
{

	add_remove_oedge(oedges, 1, e, verts[0], verts[1], verts[2]);
	add_remove_oedge(oedges, 1, e, verts[1], verts[0], verts[2]);
	add_remove_oedge(oedges, 1, e, verts[2], verts[0], verts[1]);
}

template<class V>
inline size_t
remove_vertex(oedge<V,2> *oedges, V v0, V *order, size_t top)
{

	if (oedges[v0].degree == 1) {
		V const e = oedges[v0].edge;
		V const v1 = oedges[v0].overts[0];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0);
		order[--top] = e;
	}

	return top;
}

template<class V>
inline size_t
remove_vertex(oedge<V,3> *oedges, V v0, V *order, size_t top)
{

	if (oedges[v0].degree == 1) {
		V const e = oedges[v0].edge;
		V const v1 = oedges[v0].overts[0];
		V const v2 = oedges[v0].overts[1];
		oedges[v0].degree = 0;
		remove_oedge(oedges, e, v1, v0, v2);
		remove_oedge(oedges, e, v2, v0, v1);
		order[--top] = e;
	}

	return top;
}

// fast_divide32(3) from NetBSD.
inline uint32_t
fastdiv(uint32_t val, uint64_t mul, uint8_t s2)
{
	uint8_t constexpr s1 = 1; // s1 is 1 for every partsz greater than 1.
	uint32_t const hi = (val * mul) >> 32;

	return (hi + ((val - hi) >> s1)) >> s2;
}

// fast_remainder32(3) from NetBSD.
inline uint32_t
fastrem(uint32_t val, uint32_t div, uint64_t mul, uint8_t s2)
{

	return val - div * fastdiv(val, mul, s2);
}

inline entry_iterator::entry_iterator()
	: iter(nullptr)
	, state(nullptr)
	, cur(nullptr)
{}

inline entry_iterator::entry_iterator(rgph_entry_iterator_t i, void *s)
	: iter(i)
	, state(s)
	, cur(iter(state))
{}

inline void
entry_iterator::operator++()
{

	cur = iter(state);
}

inline struct rgph_entry const *
entry_iterator::operator->() const
{

	assert(cur != nullptr);
	return cur;
}

inline struct rgph_entry const &
entry_iterator::operator*() const
{

	assert(cur != nullptr);
	return *cur;
}

inline bool
operator==(entry_iterator const &a, entry_iterator const &b)
{

	return a.cur == b.cur;
}

inline bool
operator!=(entry_iterator const &a, entry_iterator const &b)
{

	return a.cur != b.cur;
}

template<class V, int R, class H>
inline
vector_hash<V,R,H>::vector_hash(func_t f, unsigned long seed)
	: func(f)
	, seed(seed)
{}

template<class V, int R, class H>
inline V const *
vector_hash<V,R,H>::impl(void const *key, size_t keylen, V (&hashes)[4]) const
{
	static_assert(zerocopy, "Type dispatch didn't work.");

	func(key, keylen, seed, hashes);
	return hashes;
}

template<class V, int R, class H>
inline V const *
vector_hash<V,R,H>::impl(void const *key, size_t keylen, V (&hashes)[R]) const
{
	static_assert(!zerocopy, "Type dispatch didn't work.");

	H h[4]; // Some hashes are x4.

	func(key, keylen, seed, h);
	for (size_t i = 0; i < R; i++)
		hashes[i] = h[i];
	return hashes;
}

template<class V, int R, class H>
inline V const *
vector_hash<V,R,H>::operator()(void const *key, size_t keylen) const
{

	return this->impl(key, keylen, hashes); // Type dispatch.
}

template<class V, int R, class H>
inline
scalar_hash<V,R,H>::scalar_hash(func_t f, unsigned long seed)
	: func(f)
	, seed(seed)
{}

template<class V, int R, class H>
inline V const *
scalar_hash<V,R,H>::operator()(void const *key, size_t keylen) const
{
	size_t constexpr nbits = sizeof(H) * CHAR_BIT / R;
	H constexpr mask = (H(1) << nbits) - 1;

	H h = func(key, keylen, seed);
	for (size_t i = 0; i < R-1; i++)
		hashes[i] = (h >> i * nbits) & mask;
	hashes[R-1] = h >> (sizeof(H) * CHAR_BIT - nbits);

	return hashes;
}

inline
fastrem_partition::fastrem_partition(size_t nverts, size_t r)
	: partsz(nverts / r)
{
	bool constexpr branchless = true;
	uint8_t s1;

	assert(partsz > 1 && (nverts % r) == 0);
	rgph_fastdiv_prepare(partsz, &mul, &s1, &s2, branchless);
	assert(s1 == 1); // s1 is 1 for every partsz greater than 1.
}

inline uint32_t
fastrem_partition::operator()(vert_t const *h, size_t r) const
{

	return fastrem(h[r], partsz, mul, s2) + r * partsz;
}

inline
lemire_partition::lemire_partition(size_t nverts, size_t r, size_t nbits)
	: partsz(nverts / r)
	, shift(nbits > 32 * r ? 32 : nbits / r)
{

	assert(partsz > 1 && (nverts % r) == 0);
}

inline uint32_t
lemire_partition::operator()(vert_t const *h, size_t r) const
{

	return ((h[r] * (uint64_t)partsz) >> shift) + r * partsz;
}

inline size_t
bdz_assigner::operator()(vert_t, size_t i) const
{

	return i;
}

template<class X>
inline X
chm_assigner<X>::operator()(vert_t e, size_t) const
{
	bool constexpr big = sizeof(X) == sizeof(big_index_t);

	return big || index != nullptr ? index[e] : e;
}

template<class V, int R, class H>
inline scalar_hash<V,R,H>
make_hash(H (*func)(void const *, size_t, uint32_t), unsigned long seed)
{

	return scalar_hash<V,R,H>(func, seed);
}

template<class V, int R, class H>
inline vector_hash<V,R,H>
make_hash(void (*func)(void const *, size_t, uint32_t, H *), unsigned long seed)
{

	return vector_hash<V,R,H>(func, seed);
}

template<class X>
inline void
init_chm_index(X *index, size_t n)
{

	for (size_t i = 0; i < n; i++)
		index[i] = i;
}

inline void
copy_chm_index(void *new_index, void *index, size_t n)
{
	auto dst = static_cast<big_index_t *>(new_index);
	auto src = static_cast<index_t const *>(index);

	for (size_t i = 0; i < n; ++i)
		dst[i] = src[i];
}

void *
realloc_chm_index(void *index, size_t nkeys, size_t e, big_index_t indexmax)
{
	bool const big = indexmax > INDEX_MAX;
	size_t const elemsz = big ? sizeof(big_index_t) : sizeof(index_t);
	void *new_index;

	if (nkeys > SIZE_MAX / elemsz) {
		errno = ENOMEM;
		return nullptr;
	}

	new_index = malloc(elemsz * nkeys);
	if (new_index == nullptr)
		return nullptr;

	if (index != nullptr) {
		copy_chm_index(new_index, index, e);
		free(index);
	} else if (big) {
		init_chm_index(static_cast<big_index_t *>(new_index), e);
	} else {
		init_chm_index(static_cast<index_t *>(new_index), e);
	}

	return new_index;
}

template<class X, class Iter, class Hash, class Reduce, class V, int R>
inline V
init_graph(Iter &keys, Iter const &keys_end,
    Reduce const &reduce, Hash const &hash,
    edge<V,R> *edges, size_t nkeys, oedge<V,R> *oedges,
    size_t *datalenmin, size_t *datalenmax,
    big_index_t *indexmin, big_index_t *indexmax,
    V e, X *index)
{
	static_assert(sizeof(nullptr_index_t) < sizeof(index_t),
	    "Impossible to detect nullptr index at compile-time.");

	for (; e < nkeys && keys != keys_end; ++e, ++keys) {
		rgph_entry const &ent = *keys;
		big_index_t const i = ent.has_index ? ent.index : e;

		if (i > *indexmax) {
			*indexmax = i;

			/*
			 * Returns from this branch are rare:
			 *  1. to allocate index when i != e for the first time
			 *  2. to switch to the big index when i > INDEX_MAX
			 *     for the first time.
			 *
			 * Compile-time checks help to reduce runtime overhead
			 * to a single comparison.
			 */
			if (sizeof(X) == sizeof(nullptr_index_t)) {
				if (i != e)
					return e;
			} else if (sizeof(X) == sizeof(index_t)) {
				if (i > INDEX_MAX)
					return e;
			}
		}

		if (i < *indexmin)
			*indexmin = i;

		if (sizeof(X) > sizeof(nullptr_index_t))
			index[e] = i;

		if (ent.datalen > *datalenmax)
			*datalenmax = ent.datalen;
		if (ent.datalen < *datalenmin)
			*datalenmin = ent.datalen;

		V const *verts = hash(ent.key, ent.keylen);
		for (V r = 0; r < R; ++r)
			edges[e].verts[r] = reduce(verts, r);

		add_edge(oedges, e, edges[e].verts);
	}

	return e;
}

template<class Iter, class Reduce, class Hash, class V, int R>
int
init_graph(Iter &keys, Iter const &keys_end,
    Reduce const &reduce, Hash const &hash,
    edge<V,R> *edges, size_t nkeys, oedge<V,R> *oedges,
    size_t *datalenmin, size_t *datalenmax,
    void **index, big_index_t *indexmin, big_index_t *indexmax)
{
	V e = 0;

	if (*index == nullptr) {
		e = init_graph(keys, keys_end, reduce, hash,
		    edges, nkeys, oedges, datalenmin, datalenmax,
		    indexmin, indexmax,
		    e, static_cast<nullptr_index_t *>(*index));
		if (e == nkeys)
			return RGPH_SUCCESS;
		else if (keys == keys_end)
			return RGPH_NOKEY;

		*index = realloc_chm_index(*index, nkeys, e, *indexmax);
		if (*index == nullptr)
			return RGPH_NOMEM;
	}

	if (*indexmax <= INDEX_MAX) {
		void *big_index;

		e = init_graph(keys, keys_end, reduce, hash,
		    edges, nkeys, oedges, datalenmin, datalenmax,
		    indexmin, indexmax,
		    e, static_cast<index_t *>(*index));
		if (e == nkeys)
			return RGPH_SUCCESS;
		else if (keys == keys_end)
			return RGPH_NOKEY;

		big_index = realloc_chm_index(*index, nkeys, e, *indexmax);
		if (big_index == nullptr)
			return RGPH_NOMEM;
		*index = big_index;
	}

	assert (*indexmax > INDEX_MAX);

	e = init_graph(keys, keys_end, reduce, hash,
	    edges, nkeys, oedges, datalenmin, datalenmax,
	    indexmin, indexmax,
	    e, static_cast<big_index_t *>(*index));

	return e == nkeys ? RGPH_SUCCESS : RGPH_NOKEY;
}

template<class V, int R>
size_t
peel_graph(edge<V,R> *edges, size_t nkeys,
    oedge<V,R> *oedges, size_t nverts, V *order)
{
	size_t top = nkeys;

	for (V v0 = 0; v0 < nverts; ++v0)
		top = remove_vertex(oedges, v0, order, top);

	for (size_t i = nkeys; i > 0 && i > top; --i) {
		edge<V,R> const &e = edges[order[i-1]];
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

inline size_t constexpr
maxsize(size_t a, size_t b)
{

	return a > b ? a : b;
}

inline size_t constexpr
maxsize(size_t a, size_t b, size_t c)
{

	return maxsize(maxsize(a, b), c);
}

template<class V, int R>
inline size_t
duphash_size(size_t nverts)
{
	size_t const nelems = nverts / R;

	assert((nverts % R) == 0);

	if (nelems > SIZE_MAX / sizeof(duphash_entry_t))
		return 0;

	// Round up to V's size because the hash table is followed by
	// a peel index array. In practice, rounding doesn't make any
	// difference because V isn't bigger than duphash_entry_t.
	return round_up(nelems * sizeof(duphash_entry_t), sizeof(V));
}

/*
 * Memory allocated for oedges array is shared with a hash table
 * followed by a peel index.
 * It's also shared with the assign functions: chm needs X[nverts]
 * elements, bdz needs nverts bytes.
 * For typical sizes of V (vert_t) and X (big_index_t), oedges array is
 * bigger but this function calculates all 3 sizes and returns the max.
 */
template<class V, int R>
inline size_t
oedges_size_impl(size_t nkeys, size_t nverts)
{
	size_t constexpr osz = sizeof(oedge<V,R>);
	size_t constexpr vsz = sizeof(V);
	size_t constexpr xsz = sizeof(big_index_t);
	static_assert(osz > vsz, "Impossible: oedge<V,R> is made of R+1 V's.");

	assert(nverts > nkeys);

	if (nverts > SIZE_MAX / maxsize(osz, vsz, xsz))
		return 0;

	size_t const oedges_sz = nverts * osz;
	size_t const hash_sz = duphash_size<V,R>(nverts);
	if (hash_sz == 0)
		return 0;

	size_t const peel_index_sz = nkeys * vsz;
	if (peel_index_sz > SIZE_MAX - hash_sz)
		return 0;

	return maxsize(oedges_sz, hash_sz + peel_index_sz, nverts * xsz);
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

inline size_t
graph_max_keys(unsigned int flags)
{
	int const rank = graph_rank(flags);
	size_t const nbits = hash_bits(flags);

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
graph_nverts(int *flags, size_t nkeys)
{
	size_t const max_nkeys = graph_max_keys(*flags);

	if (nkeys == 0 || nkeys > max_nkeys)
		return 0;

	int const rank = graph_rank(*flags);
	// nverts is approx. 2.125*nkeys for rank 2 and 1.25*nkeys for rank 3.
	int const scale = 4 - rank;
	int const fract = 1 << (5 - rank);
	size_t const nv = scale * nkeys + round_up(nkeys, fract) / fract;
	size_t const nverts = maxsize(round_up(nv, rank), 24);

	assert(nverts > nkeys && nverts > scale * nkeys); // No overflow.

	return nverts;
}

// The destination array is often called g in computer science literature.
template<class G, class V, int R, class A>
inline void
assign(edge<V,R> const *edges, V const *order, size_t nkeys,
    A assigner, G *g, size_t nverts, int *assigned)
{
	size_t const wbits = sizeof(assigned[0]) * CHAR_BIT;

#define ASSIGN(v) assigned[v / wbits] |= 1 << (v % wbits)
#define IS_ASSIGNED(v) (((assigned[v / wbits] >> (v % wbits)) & 1) != 0)

	memset(assigned, 0, sizeof(assigned[0]) * ((nverts - 1) / wbits + 1));

	for (size_t i = 0; i < nkeys; i++) {
		V const e = order[i];
		assert(e < nkeys);

		for (size_t j = 0; j < R; j++) {
			V const v = edges[e].verts[j];
			assert(v < nverts);

			if (IS_ASSIGNED(v))
				continue;

			g[v] = assigner(e, j); // j for bdz and index[e] for chm
			ASSIGN(v);

			for (size_t k = 1; k < R; k++) {
				V const u = edges[e].verts[(j + k) % R];
				assert(u != v);

				if (!IS_ASSIGNED(u)) {
					// Assign all traversed vertices:
					g[u] = 0;
					ASSIGN(u);
				} else {
					// g[v] = (g[v] - g[u]) mod 2^(32|64)
					g[v] -= g[u];
				}
			}
		}
	}

	for (size_t v = 0; v < nverts; v++) {
		if (!IS_ASSIGNED(v))
			g[v] = 0;
	}

#undef IS_ASSIGNED
#undef ASSIGN
}

// In many cases, the "assigned" bitset can be replaced with
// a designated "unassigned" value.
template<class G, class V, int R, class A>
inline void
assign(edge<V,R> const *edges, V const *order, size_t nkeys,
    A assigner, G *g, size_t nverts, size_t min, size_t max)
{
	G const unassigned = max - min + 1;
	assert(unassigned != min);

	for (size_t v = 0; v < nverts; v++)
		g[v] = unassigned;

	for (size_t i = 0; i < nkeys; i++) {
		V const e = order[i];
		assert(e < nkeys);

		for (size_t j = 0; j < R; j++) {
			V const v = edges[e].verts[j];
			assert(v < nverts);

			if (g[v] != unassigned)
				continue;

			g[v] = assigner(e, j); // j for bdz and index[e] for chm
			assert(g[v] < unassigned);

			for (size_t k = 1; k < R; k++) {
				V const u = edges[e].verts[(j + k) % R];
				assert(u != v);

				if (g[u] == unassigned) {
					// Assign all traversed vertices:
					g[u] = 0;
				} else {
					// g[v] = (g[v] - g[u]) mod unassigned:
					if (g[v] < g[u])
						g[v] += unassigned;
					g[v] -= g[u];
				}
			}

			assert(g[v] < unassigned);
		}
	}

	for (size_t v = 0; v < nverts; v++) {
		if (g[v] == unassigned)
			g[v] = 0;
	}
}

inline bool
need_assigned_bitset(int flags, big_index_t indexmin, big_index_t indexmax)
{
	int const compact = flags & RGPH_INDEX_COMPACT;

	if ((compact && indexmax - indexmin < INDEX_MAX) ||
	    indexmax <= INDEX_MAX / 2)
		return false;

	return (!compact && indexmax > BIG_INDEX_MAX / 2)
	    || indexmax - indexmin == BIG_INDEX_MAX;
}

template<class V, int R>
int
build_graph(struct rgph_graph *g,
    rgph_entry_iterator_t keys, void *state, unsigned long seed)
{
	typedef edge<V,R> edge_t;
	typedef oedge<V,R> oedge_t;

	auto order = static_cast<V *>(g->order);
	auto edges = static_cast<edge_t *>(g->edges);
	auto oedges = static_cast<oedge_t *>(g->shared.oedges);
	size_t const nkeys = g->nkeys;
	size_t const nverts = g->nverts;
	unsigned int const flags = g->flags;
	size_t const nbits = hash_bits(flags);
	int res = RGPH_INVAL;

	if ((flags & ZEROED) == 0) {
		memset(order, 0, sizeof(V) * nkeys);
		memset(edges, 0, sizeof(edge_t) * nkeys);
		memset(oedges, 0, sizeof(oedge_t) * nverts);
	}

	g->core_size = nkeys;
	g->datalenmin = SIZE_MAX;
	g->datalenmax = 0;
	g->seed = seed;
	g->flags &= PUBLIC_FLAGS; // Reset internal flags.

	entry_iterator keys_start(keys, state), keys_end;

	switch (flags & (RGPH_HASH_MASK | RGPH_REDUCE_MASK)) {
	case RGPH_HASH_JENKINS2V|RGPH_REDUCE_MOD:
		res = init_graph(keys_start, keys_end,
		    fastrem_partition(nverts, R),
		    make_hash<V,R>(&rgph_u32x3_jenkins2v_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_MURMUR32V|RGPH_REDUCE_MOD:
		res = init_graph(keys_start, keys_end,
		    fastrem_partition(nverts, R),
		    make_hash<V,R>(&rgph_u32x4_murmur32v_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_MURMUR32S|RGPH_REDUCE_MOD:
		res = init_graph(keys_start, keys_end,
		    fastrem_partition(nverts, R),
		    make_hash<V,R>(&rgph_u32_murmur32s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_XXH32S|RGPH_REDUCE_MOD:
		res = init_graph(keys_start, keys_end,
		    fastrem_partition(nverts, R),
		    make_hash<V,R>(&rgph_u32_xxh32s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_XXH64S|RGPH_REDUCE_MOD:
		res = init_graph(keys_start, keys_end,
		    fastrem_partition(nverts, R),
		    make_hash<V,R>(&rgph_u64_xxh64s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_JENKINS2V|RGPH_REDUCE_MUL:
		res = init_graph(keys_start, keys_end,
		    lemire_partition(nverts, R, nbits),
		    make_hash<V,R>(&rgph_u32x3_jenkins2v_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_MURMUR32V|RGPH_REDUCE_MUL:
		res = init_graph(keys_start, keys_end,
		    lemire_partition(nverts, R, nbits),
		    make_hash<V,R>(&rgph_u32x4_murmur32v_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_MURMUR32S|RGPH_REDUCE_MUL:
		res = init_graph(keys_start, keys_end,
		    lemire_partition(nverts, R, nbits),
		    make_hash<V,R>(&rgph_u32_murmur32s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_XXH32S|RGPH_REDUCE_MUL:
		res = init_graph(keys_start, keys_end,
		    lemire_partition(nverts, R, nbits),
		    make_hash<V,R>(&rgph_u32_xxh32s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	case RGPH_HASH_XXH64S|RGPH_REDUCE_MUL:
		res = init_graph(keys_start, keys_end,
		    lemire_partition(nverts, R, nbits),
		    make_hash<V,R>(&rgph_u64_xxh64s_data, seed),
		    edges, nkeys, oedges,
		    &g->datalenmin, &g->datalenmax,
		    &g->index, &g->indexmin, &g->indexmax);
		break;
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}

	if (res != RGPH_SUCCESS)
		return res;

	g->core_size = peel_graph(edges, nkeys, oedges, nverts, order);

	g->flags |= BUILT;
	return g->core_size == 0 ? RGPH_SUCCESS : RGPH_AGAIN;
}

template<class V, int R>
V const *
build_peel_index(struct rgph_graph *g)
{
	size_t const hash_sz = duphash_size<V,R>(g->nverts);
	auto peel = reinterpret_cast<V *>(g->shared.bdz_assignments + hash_sz);

	assert(hash_sz != 0);

	if (!(g->flags & PEELED)) {
		auto order = static_cast<V const *>(g->order);

		g->flags |= PEELED;
		g->flags &= ~ASSIGNED;
		memset(peel, 0, sizeof(V) * g->nkeys);

		for (size_t i = g->nkeys; i > g->core_size; i--) {
			assert(peel[order[i-1]] == 0);
			peel[order[i-1]] = g->nkeys - i + 1;
		}
	}

	return peel;
}

template<class V, int R>
int
copy_edge(struct rgph_graph *g, size_t e, unsigned long *to, size_t *peel_order)
{
	typedef edge<V,R> edge_t;

	auto edges = static_cast<edge_t *>(g->edges);

	for (size_t r = 0; r < R; r++)
		to[r] = edges[e].verts[r];

	if (peel_order != nullptr) {
		V const *peel = build_peel_index<V,R>(g);
		*peel_order = peel[e];
	}

	return RGPH_SUCCESS;
}

template<class V, int R>
int
find_duplicates(struct rgph_graph *g,
    rgph_entry_iterator_t iter, void *state, size_t *dup)
{
	typedef edge<V,R> edge_t;

	assert((g->nverts % R) == 0);

	size_t const hash_sz = g->nverts / R;
	size_t const maxfill = g->nverts / 4; // Fill factor is 50% or 75%.
	auto hash = g->shared.duphash;

	for (size_t i = 0; i < hash_sz; i++)
		hash[i] = nullptr;

	int res = RGPH_NOKEY;
	V const *peel = build_peel_index<V,R>(g);
	auto edges = static_cast<edge_t const *>(g->edges);
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

		void const *key = keys->key;
		size_t const keylen = keys->keylen;
		V const v0 = edges[e].verts[0];

		assert(v0 < hash_sz);

		// Linear probing with a wrap-around.
		size_t v = v0, vend = hash_sz;
		for (int i = 0; i < 2; i++) {
			for (; v < vend && hash[v] != nullptr; v++) {
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

		if (hash[v] != nullptr) {
			res = RGPH_AGAIN; // Hash table is full.
			goto out;
		}

		if (keylen > SIZE_MAX - 2 * sizeof(hash[v][0])) {
			res = RGPH_NOMEM;
			goto out;
		}

		hash[v] = static_cast<duphash_entry_t>(
		    malloc(2 * sizeof(hash[v][0]) + keylen));
		if (hash[v] == nullptr) {
			res = RGPH_NOMEM;
			goto out;
		}

		hash[v][0] = e;
		hash[v][1] = keylen;
		memcpy(&hash[v][2], key, keylen);
	}
out:
	for (size_t i = 0; i < hash_sz; i++) {
		if (hash[i] != nullptr)
			free(hash[i]);
	}

	if (res == RGPH_NOMEM)
		errno = ENOMEM;

	return res;
}

template<class V, int R>
int
graph_assign_bdz(struct rgph_graph *g)
{
	typedef edge<V,R> edge_t;

	auto order = static_cast<V const *>(g->order);
	auto edges = static_cast<edge_t const *>(g->edges);
	auto assignments = g->shared.bdz_assignments;
	bdz_assigner const assigner;

	assert(g->core_size == 0);

	g->flags &= ~(PEELED|ASSIGNED);

	assign(edges, order, g->nkeys, assigner,
	    assignments, g->nverts, 0, R - 1);

	g->flags |= ASSIGNED;
	return RGPH_SUCCESS;
}

template<class V, int R, class X>
int
graph_assign_chm(struct rgph_graph *g)
{
	typedef edge<V,R> edge_t;

	auto order = static_cast<V const *>(g->order);
	auto index = static_cast<X const *>(g->index);
	auto edges = static_cast<edge_t const *>(g->edges);
	auto assignments = static_cast<X *>(g->shared.chm_assignments);
	chm_assigner<X> const assigner = { index };

	assert(sizeof(X) == sizeof(index_t) || index != nullptr);
	assert(g->core_size == 0);

	if (need_assigned_bitset(g->flags, g->indexmin, g->indexmax)) {
		if (g->assigned == nullptr) {
			size_t const wsize = sizeof(g->assigned[0]);
			size_t const wbits = wsize * CHAR_BIT;
			size_t const nwords = (g->nverts - 1) / wbits + 1;

			g->assigned = (int *)malloc(wsize * nwords);
			if (g->assigned == nullptr)
				return RGPH_NOMEM;
		}
		assign(edges, order, g->nkeys, assigner,
		    assignments, g->nverts, g->assigned);
	} else if (g->flags & RGPH_INDEX_COMPACT) {
		assign(edges, order, g->nkeys, assigner,
		    assignments, g->nverts, g->indexmin, g->indexmax);
	} else {
		big_index_t const l = 1;
		big_index_t const indexmax = (l << fls64(g->indexmax)) - l;

		assign(edges, order, g->nkeys, assigner,
		    assignments, g->nverts, 0, indexmax);
	}

	g->flags |= ASSIGNED;
	g->flags &= ~PEELED;

	return RGPH_SUCCESS;
}

template<class V, int R>
int
graph_assign(struct rgph_graph *g)
{

	switch (g->flags & RGPH_ALGO_MASK) {
	case RGPH_ALGO_BDZ:
		return graph_assign_bdz<V,R>(g);
	case RGPH_ALGO_CHM:
		return (g->indexmax > INDEX_MAX)
		    ? graph_assign_chm<V,R,big_index_t>(g)
		    : graph_assign_chm<V,R,index_t>(g);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

inline bool
set_default_flags(int *flags)
{

	if (*flags & ~PUBLIC_FLAGS)
		return false;

	// Fail if both RGPH_RANK2 and RGPH_RANK3 are passed.
	if ((*flags & RGPH_RANK_MASK) == RGPH_RANK_MASK)
		return false;

	// Don't accept bad hash flags.
	if ((*flags & RGPH_HASH_MASK) > RGPH_HASH_LAST)
		return false;

	// Fail if both RGPH_ALGO_CHM and RGPH_ALGO_BDZ are passed.
	if ((*flags & RGPH_ALGO_MASK) == RGPH_ALGO_MASK)
		return false;

	// Fail if both RGPH_REDUCE_MOD and RGPH_REDUCE_MUL are passed.
	if ((*flags & RGPH_REDUCE_MASK) == RGPH_REDUCE_MASK)
		return false;

	// Fail if both RGPH_INDEX_COMPACT and RGPH_INDEX_SPARSE are passed.
	if ((*flags & RGPH_INDEX_MASK) == RGPH_INDEX_MASK)
		return false;

	if ((*flags & RGPH_RANK_MASK) == RGPH_RANK_DEFAULT)
		*flags |= RGPH_RANK3;

	if ((*flags & RGPH_HASH_MASK) == RGPH_HASH_DEFAULT)
		*flags |= RGPH_HASH_JENKINS2V;

	if ((*flags & RGPH_ALGO_MASK) == RGPH_ALGO_DEFAULT)
		*flags |= RGPH_ALGO_CHM;

	if ((*flags & RGPH_REDUCE_MASK) == RGPH_REDUCE_DEFAULT)
		*flags |= RGPH_REDUCE_MOD;

	if ((*flags & RGPH_INDEX_MASK) == RGPH_INDEX_DEFAULT)
		*flags |= RGPH_INDEX_COMPACT;

	return true;
}

inline bool
check_flags(int flags)
{

	return set_default_flags(&flags);
}

inline bool
flags_changed(unsigned int *flags, int new_flags, unsigned int mask)
{
	unsigned int const select_flags = new_flags & mask;

	return select_flags != 0 && select_flags != (*flags & mask);
}

bool
update_flags_for_build(unsigned int *flags, int new_flags, size_t nkeys)
{

	if (!check_flags(new_flags))
		return false;

	if (flags_changed(flags, new_flags, RGPH_RANK_MASK))
		return false;

	if ((new_flags & RGPH_HASH_MASK) != RGPH_HASH_DEFAULT) {
		if (nkeys > graph_max_keys(new_flags))
			return false;

		*flags &= ~RGPH_HASH_MASK;
		*flags |= new_flags & RGPH_HASH_MASK;
	}

	if ((new_flags & RGPH_ALGO_MASK) != RGPH_ALGO_DEFAULT) {
		*flags &= ~RGPH_ALGO_MASK;
		*flags |= new_flags & RGPH_ALGO_MASK;
	}

	if ((new_flags & RGPH_REDUCE_MASK) != RGPH_REDUCE_DEFAULT) {
		*flags &= ~RGPH_REDUCE_MASK;
		*flags |= new_flags & RGPH_REDUCE_MASK;
	}

	if ((new_flags & RGPH_INDEX_MASK) != RGPH_INDEX_DEFAULT) {
		*flags &= ~RGPH_INDEX_MASK;
		*flags |= new_flags & RGPH_INDEX_MASK;
	}

	return true;
}

bool
update_flags_for_assign(unsigned int *flags, int new_flags)
{

	if (!check_flags(new_flags))
		return false;

	if (flags_changed(flags, new_flags, RGPH_RANK_MASK))
		return false;

	if (flags_changed(flags, new_flags, RGPH_HASH_MASK))
		return false;

	if (flags_changed(flags, new_flags, RGPH_REDUCE_MASK))
		return false;

	if ((new_flags & RGPH_ALGO_MASK) != RGPH_ALGO_DEFAULT) {
		*flags &= ~RGPH_ALGO_MASK;
		*flags |= new_flags & RGPH_ALGO_MASK;
	}

	if ((new_flags & RGPH_INDEX_MASK) != RGPH_INDEX_DEFAULT) {
		*flags &= ~RGPH_INDEX_MASK;
		*flags |= new_flags & RGPH_INDEX_MASK;
	}

	return true;
}

} // anon namespace

extern "C"
void
rgph_free_graph(struct rgph_graph *g)
{

	free(g->assigned);
	free(g->index);
	free(g->shared.oedges);
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

	if (!set_default_flags(&flags)) {
		errno = EINVAL;
		return nullptr;
	}

	nverts = graph_nverts(&flags, nkeys);
	if (nverts == 0) {
		errno = ERANGE;
		return nullptr;
	}

	assert(nverts > nkeys);

	r = graph_rank(flags);
	esz = edge_size(r);
	if (esz == 0) {
		errno = EINVAL;
		return nullptr;
	}

	osz = oedges_size(r, nkeys, nverts);
	if (osz == 0) {
		errno = ENOMEM;
		return nullptr;
	}

	g = static_cast<struct rgph_graph *>(calloc(sizeof(*g), 1));
	if (g == nullptr)
		return nullptr;

	g->order         = nullptr;
	g->edges         = nullptr;
	g->shared.oedges = nullptr;
	g->index         = nullptr;
	g->assigned      = nullptr;

	g->order = calloc(sizeof(vert_t), nkeys);
	if (g->order == nullptr)
		goto err;

	g->edges = calloc(esz, nkeys);
	if (g->edges == nullptr)
		goto err;

	g->shared.oedges = calloc(osz, 1);
	if (g->shared.oedges == nullptr)
		goto err;

	g->seed       = 0;
	g->nkeys      = nkeys;
	g->nverts     = nverts;
	g->core_size  = nkeys;
	g->datalenmin = SIZE_MAX;
	g->datalenmax = 0;
	g->indexmin = BIG_INDEX_MAX;
	g->indexmax = 0;
	g->flags      = flags | ZEROED; // calloc

	return g;
err:
	save_errno = errno;
	rgph_free_graph(g);
	errno = save_errno;
	return nullptr;
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
uint64_t
rgph_index_min(struct rgph_graph *g)
{

	return g->indexmin;
}

extern "C"
uint64_t
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
	int const res = (g->flags & ASSIGNED) != 0;

	assert(res != ((g->flags & PEELED) != 0));
	return res;
}

extern "C"
int
rgph_build_graph(struct rgph_graph *g, int flags,
    unsigned long seed, rgph_entry_iterator_t keys, void *state)
{

	if (!update_flags_for_build(&g->flags, flags, g->nkeys))
		return RGPH_INVAL;

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
rgph_assign(struct rgph_graph *g, int flags)
{

	if (!(g->flags & BUILT))
		return RGPH_INVAL;
	if (g->core_size != 0)
		return RGPH_AGAIN;
	if (!update_flags_for_assign(&g->flags, flags))
		return RGPH_INVAL;

	switch (graph_rank(g->flags)) {
	case 2:
		return graph_assign<vert_t,2>(g);
	case 3:
		return graph_assign<vert_t,3>(g);
	default:
		assert(0 && "rgph_alloc_graph() should have caught it");
		return RGPH_INVAL;
	}
}

extern "C"
void const *
rgph_assignments(struct rgph_graph *g, size_t *width)
{
	bool const big = (g->indexmax > INDEX_MAX);
	bool const bdz = (g->flags & RGPH_ALGO_BDZ) != 0;
	bool const assigned = (g->flags & ASSIGNED) != 0;

	if (assigned && width != nullptr)
		*width = bdz ? 1 : big ? sizeof(big_index_t) : sizeof(index_t);

	assert(g->shared.chm_assignments == g->shared.bdz_assignments);

	return assigned ? g->shared.chm_assignments : nullptr;
}

extern "C"
int
rgph_copy_assignment(struct rgph_graph *g, size_t n, unsigned long long *to)
{
	unsigned int const flags = g->flags;
	bool const assigned = (flags & ASSIGNED) != 0;
	void const *assignments = g->shared.oedges;

	if (!assigned)
		return RGPH_INVAL;

	if (n >= g->nverts)
		return RGPH_RANGE;

	switch (flags & RGPH_ALGO_MASK) {
	case RGPH_ALGO_BDZ:
		*to = static_cast<uint8_t const *>(assignments)[n];
		return RGPH_SUCCESS;
	case RGPH_ALGO_CHM:
		if (g->indexmax > INDEX_MAX)
			*to = static_cast<big_index_t const *>(assignments)[n];
		else
			*to = static_cast<index_t const *>(assignments)[n];
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
