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

 #include <stddef.h>

/*
 * The algorithm below is based on paper
 * Cache-Oblivious Peeling of Random Hypergraphs by Djamal Belazzougui,
 * Paolo Boldi, Giuseppe Ottaviano, Rossano Venturini, and Sebastiano
 * Vigna.
 * http://zola.di.unipi.it/rossano/wp-content/papercite-data/pdf/dcc14.pdf
 */
namespace {

template<class T, int R>
struct edge {
	T verts[R];
	T idx; // XXX
};

/*
 * Data type for a valid oriented edge (v0, v1, v2), v1 < v2.
 * The first vertex v0 is implicit and is determined by an index
 * of the corresponding element in the state->oedges array.
 * If the degree of v0 is greater than 1, other members don't make
 * sense because they're a result of XORing multiple random values.
 */
template<class T, int R>
struct oedge {
	T verts[R-1]; // v1 (and v2, if R==3).
	T degree;     // Degree of v0.
	T edge;
};

template<class T>
inline void
add_remove_edge(oedge<T,2> *oedges, int delta, T e, const T *verts)
{
	const T v0 = verts[0];
	const T v1 = verts[1];

	oedges[v0].verts[0] ^= v1;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class T>
inline void
remove_edge(oedge<T,2> *oedges, T e, T v0, T v1)
{
	const T verts[2] = { v0, v1 };

	return add_remove_edge(oedges, -1, e, verts);
}

template<class T>
inline void
add_remove_edge(oedge<T,3> *oedges, int delta, T e, const T *verts)
{
	const T v0 = verts[0];
	const T v1 = verts[1];
	const T v2 = verts[2];

	oedges[v0].verts[v1 < v2 ? 0 : 1] ^= v1;
	oedges[v0].verts[v1 < v2 ? 1 : 0] ^= v2;
	oedges[v0].degree += delta;
	oedges[v0].edge ^= e;
}

template<class T>
inline void
remove_edge(oedge<T,3> *oedges, T e, T v0, T v1, T v2)
{
	const T verts[3] = { v0, v1, v2 };

	return add_remove_edge(oedges, -1, e, verts);
}

template<class T, int R>
inline void
add_edge(oedge<T,R> *oedges, T e, const T *verts)
{

	return add_remove_edge(oedges, 1, e, verts);
}

template<class T>
inline size_t
remove_vertex(oedge<T,2> *oedges, T v0, T *order, size_t end)
{

	if (oedges[v0].degree == 1) {
		const T e = oedges[v0].edge;
		const T v1 = oedges[v0].verts[0];
		oedges[v0].degree = 0;
		remove_edge(oedges, e, v1, v0);
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
		const T v1 = oedges[v0].verts[0];
		const T v2 = oedges[v0].verts[1];
		oedges[v0].degree = 0;
		remove_edge(oedges, e, v1, v0, v2);
		remove_edge(oedges, e, v2, v0, v1);
		order[--end] = e;
	}

	return end;
}

} // namespace
