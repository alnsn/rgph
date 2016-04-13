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
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "rgph.h"
#include "rgph_fastdiv.h"
#include "rgph_hash.h"
#include "rgph_hash_impl.h"

#define GRAPH_MT  "rgph.graph"
#define ASSIGN_MT "rgph.assign"

/* rgph.edges("peel,key", iter) => EDGES_PEEL + (EDGES_KEY << EDGES_SHIFT). */
#define EDGES_PEEL  1 /* 001, iter is optional */
#define EDGES_KEY   2 /* 010, requires iter    */
#define EDGES_INDEX 4 /* 100, requires iter    */
#define EDGES_SHIFT 3
#define EDGES_PEEL_MASK 0x09249249 /* 0000,1001,0010,0100,1001,0010,0100,1001 */
#define EDGES_ITER_MASK 0x36db6db6 /* 0011,0110,1101,1011,0110,1101,1011,0110 */

struct build_iter_state {
	struct rgph_entry ent;
	lua_State *L;
	int top;  /* Index of an iterator state on the stack. */
};

struct flag_str {
	int flag;
	int mask;
	const char *str;
};

static const struct flag_str flag_strings[] = {
	{ RGPH_HASH_JENKINS2V, RGPH_HASH_MASK, "jenkins2v" },
	{ RGPH_HASH_MURMUR32V, RGPH_HASH_MASK, "murmur32v" },
	{ RGPH_HASH_MURMUR32S, RGPH_HASH_MASK, "murmur32s" },
	{ RGPH_HASH_XXH32S,    RGPH_HASH_MASK, "xxh32s"    },
	{ RGPH_HASH_XXH64S,    RGPH_HASH_MASK, "xxh64s"    },
	{ RGPH_RANK2,          RGPH_RANK_MASK, "rank2"     },
	{ RGPH_RANK3,          RGPH_RANK_MASK, "rank3"     },
	{ RGPH_ALGO_CHM,       RGPH_ALGO_MASK, "chm"       },
	{ RGPH_ALGO_BDZ,       RGPH_ALGO_MASK, "bdz"       },
	{ RGPH_DIV_POW2,       RGPH_DIV_MASK,  "pow2"      },
	{ RGPH_DIV_FAST,       RGPH_DIV_MASK,  "fastdiv"   },
	/* XXX { RGPH_INDEX_XXX } */
};

static const char *algo_strings[] = { "", "chm", "bdz" };


static const struct flag_str *
find_flag(int flag)
{
	const size_t nflags = sizeof(flag_strings) / sizeof(flag_strings[0]);
	size_t i;

	for (i = 0; i < nflags; i++) {
		if ((flag & flag_strings[i].mask) == flag_strings[i].flag)
			return &flag_strings[i];
	}

	return NULL;
}

static const int *
find_flag_by_name(const char *str)
{
	const size_t nflags = sizeof(flag_strings) / sizeof(flag_strings[0]);
	size_t i;

	for (i = 0; i < nflags; i++) {
		if (strcmp(str, flag_strings[i].str) == 0)
			return &flag_strings[i].flag;
	}

	return NULL;
}

static void
setuservalue(lua_State *L, int index)
{

#if LUA_VERSION_NUM <= 501
	lua_setfenv(L, index);
#else
	lua_setuservalue(L, index);
#endif
}

static int
parse_flags(lua_State *L, int arg)
{
	char buf[64];
	char *tok, *last_tok;
	const char *sep = "+|,";
	const char *opts;
	size_t optslen = 0;
	const int *pflag;
	int flags = 0; /* aka RGPH_DEFAULT */

	opts = luaL_optlstring(L, arg, "", &optslen);
	if (optslen > sizeof(buf) - 1)
		return luaL_argerror(L, arg, "too long");

	memcpy(buf, opts, optslen);
	buf[optslen] = '\0';

	for (tok = strtok_r(buf, sep, &last_tok); tok != NULL;
	    tok = strtok_r(NULL, sep, &last_tok)) {
		pflag = find_flag_by_name(tok);
		if (pflag == NULL)
			return luaL_argerror(L, arg, "parse error");
		flags |= *pflag;
	}

	return flags;
}

static int
new_graph_fn(lua_State *L)
{
	struct rgph_graph **pg;
	lua_Integer nkeys;
	int flags;

	nkeys = luaL_checkinteger(L, 1);
	if (nkeys < 0)
		return luaL_argerror(L, 1, "not in range");

	flags = parse_flags(L, 2);

	pg = (struct rgph_graph **)lua_newuserdata(L, sizeof(pg));
	*pg = NULL;
	luaL_getmetatable(L, GRAPH_MT);
	lua_setmetatable(L, -2);

	*pg = rgph_alloc_graph(nkeys, flags);
	if (*pg == NULL) {
		switch (errno) {
		case ERANGE:
			return luaL_argerror(L, 1, "out of range");
		case EINVAL:
			return luaL_argerror(L, 2, "invalid flags");
		case ENOMEM:
			return luaL_error(L, "not enough memory");
		default:
			return luaL_error(L, "errno %d", errno);
		}
	}

	return 1;
}

static int
graph_gc(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg != NULL)
		rgph_free_graph(*pg);
	*pg = NULL;
	return 0;
}

static int
graph_rank(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_rank(*pg));
	return 1;
}

static int
graph_algo(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushstring(L, find_flag(rgph_flags(*pg) & RGPH_ALGO_MASK)->str);
	return 1;
}

static int
graph_hash(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushstring(L, find_flag(rgph_flags(*pg) & RGPH_HASH_MASK)->str);
	return 1;
}

static int
graph_hash_bits(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_hash_bits(*pg));
	return 1;
}

static int
graph_entries(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_entries(*pg));
	return 1;
}

static int
graph_vertices(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_vertices(*pg));
	return 1;
}

static int
graph_datalen_min(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_datalen_min(*pg));
	return 1;
}

static int
graph_datalen_max(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_datalen_max(*pg));
	return 1;
}

static int
graph_core_size(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_core_size(*pg));
	return 1;
}

static int
graph_flags(lua_State *L)
{
	luaL_Buffer buf;
	char *dst;
	struct rgph_graph **pg;
	const char *delim;
	const size_t nflags = sizeof(flag_strings) / sizeof(flag_strings[0]);
	size_t i, flen, dlen, dcopy = 0;
	int flags;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	delim = luaL_optlstring(L, 2, ",", &dlen);

	flags = rgph_flags(*pg);

	luaL_buffinit(L, &buf);

	for (i = 0; i < nflags; i++) {
		if ((flags & flag_strings[i].mask) == flag_strings[i].flag) {
			flen = strlen(flag_strings[i].str);
			dst = luaL_prepbuffer(&buf);

			if (dcopy > 0) {
				if (dcopy > LUAL_BUFFERSIZE - flen)
					return luaL_argerror(L, 2, "too long");
				memcpy(dst, delim, dcopy);
			}

			memcpy(dst + dcopy, flag_strings[i].str, flen);
			luaL_addsize(&buf, dcopy + flen);
			dcopy = dlen;
		}
	}

	luaL_pushresult(&buf);
	return 1;
}

static int
graph_div_hint(lua_State *L)
{
	struct rgph_graph **pg;
	int flags;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	flags = rgph_flags(*pg);

	if (flags & RGPH_DIV_POW2)
		lua_pushstring(L, "pow2");
	else if (flags & RGPH_DIV_FAST)
		lua_pushstring(L, "fastdiv");
	else
		lua_pushstring(L, "");

	return 1;
}

static int
graph_seed(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushinteger(L, rgph_seed(*pg));
	return 1;
}

static struct rgph_entry *
graph_build_iter(void *raw_state)
{
	struct build_iter_state *state = (struct build_iter_state *)raw_state;
	lua_State *L = state->L;
	const int top = state->top;

	lua_settop(L, top); /* GC data and index from the previous iteration. */

	lua_pushvalue(L, top - 2); /* Push the iterator function. */
	lua_pushvalue(L, top - 1); /* Push the state.             */
	lua_pushvalue(L, top - 0); /* Push the first var.         */

	/* Call the iterator function and return 3 vars; may throw Lua error. */
	lua_call(L, 2, 3);
	lua_remove(L, top); /* Move the first var to its place. */

	/* XXX don't convert to string. */
	state->ent.key  = lua_tolstring(L, top + 0, &state->ent.keylen);
	state->ent.data = lua_tolstring(L, top + 1, &state->ent.datalen);
	state->ent.index = lua_isnil(L, top + 2) ?
	    SIZE_MAX : (size_t)lua_tointeger(L, top + 2);
	return state->ent.key == NULL ? NULL : &state->ent;
}

static int
count_keys_fn(lua_State *L)
{
	struct build_iter_state state;
	const int nargs = 1;

	luaL_checkany(L, nargs); /* Will check later if it's callable. */

	state.L = L;
	state.top = nargs + 2; /* Iterator state and the first var. */

	lua_pushinteger(L, rgph_count_keys(&graph_build_iter, &state));
	return 1;
}

static int
fastdiv_prepare_fn(lua_State *L)
{
	uint32_t div, mul;
	int nbits, inc;
	uint8_t s1, s2;

	div = luaL_checkinteger(L, 1);
	nbits = luaL_optinteger(L, 2, 0);

	if (nbits + 0u > sizeof(div) * CHAR_BIT)
		return luaL_argerror(L, 2, "out of range");

	rgph_fastdiv_prepare(div, &mul, &s1, &s2, nbits, &inc);

	lua_pushinteger(L, mul);
	lua_pushinteger(L, s1);
	lua_pushinteger(L, s2);

	if (nbits > 0)
		lua_pushinteger(L, inc);

	return nbits > 0 ? 4 : 3;
}

static int
jenkins2v_fn(lua_State *L)
{
	uint32_t h[3], seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	rgph_u32x3_jenkins2v_data(str, len, seed, h);

	lua_pushinteger(L, h[0]);
	lua_pushinteger(L, h[1]);
	lua_pushinteger(L, h[2]);

	return 3;
}

static int
murmur32v_fn(lua_State *L)
{
	uint32_t h[4], seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	rgph_u32x4_murmur32v_data(str, len, seed, h);

	lua_pushinteger(L, h[0]);
	lua_pushinteger(L, h[1]);
	lua_pushinteger(L, h[2]);
	lua_pushinteger(L, h[3]);

	return 4;
}

static int
murmur32s_fn(lua_State *L)
{
	uint32_t seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	lua_pushinteger(L, rgph_u32_murmur32s_data(str, len, seed));

	return 1;
}

static int
xxh32s_fn(lua_State *L)
{
	uint32_t seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	lua_pushinteger(L, rgph_u32_xxh32s_data(str, len, seed));

	return 1;
}

static int
xxh64s_fn(lua_State *L)
{
	uint64_t h;
	uint32_t seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	h = rgph_u64_xxh64s_data(str, len, seed);

	lua_pushinteger(L, h & 0xffffffffu);
	lua_pushinteger(L, h >> 32);

	return 2;
}

static int
graph_build(lua_State *L)
{
	struct build_iter_state state;
	struct rgph_graph **pg;
	unsigned long seed;
	const int nargs = 3;
	int res;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	seed = luaL_checkinteger(L, 2);
	luaL_checkany(L, nargs); /* Will check later if it's callable. */

	state.L = L;
	state.top = nargs + 2; /* Iterator state and the first var. */

	res = rgph_build_graph(*pg, seed, &graph_build_iter, &state);

	lua_pushboolean(L, res == RGPH_SUCCESS);
	switch (res) {
	case RGPH_AGAIN:
	case RGPH_SUCCESS:
		return 1;
	case RGPH_INVAL:
		lua_pushstring(L, "invalid value");
		return 2;
	case RGPH_NOKEY:
		lua_pushstring(L, "iterator returned no key");
		return 2;
	default:
		lua_pushfstring(L, "unknown error %d", res);
		return 2;
	}
}

static int
graph_find_duplicates(lua_State *L)
{
	struct build_iter_state state;
	struct rgph_graph **pg;
	size_t dup[2];
	const int nargs = 2;
	int res;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	luaL_checkany(L, nargs); /* Will check later if it's callable. */

	state.L = L;
	state.top = nargs + 2; /* Iterator state and the first var. */

	res = rgph_find_duplicates(*pg, &graph_build_iter, &state, dup);

	lua_pushboolean(L, res == RGPH_SUCCESS);
	switch (res) {
	case RGPH_SUCCESS:
		lua_pushinteger(L, dup[0]);
		lua_pushinteger(L, dup[1]);
		return 3;
	case RGPH_AGAIN:
		lua_pushstring(L, "try again");
		return 2;
	case RGPH_NOKEY:
		lua_pushstring(L, "no duplicates");
		return 2;
	case RGPH_NOMEM:
		lua_pushstring(L, "not enough memory");
		return 2;
	case RGPH_INVAL:
		lua_pushstring(L, "invalid value");
		return 2;
	default:
		lua_pushfstring(L, "unknown error %d", res);
		return 2;
	}
}

static int
graph_assign(lua_State *L)
{
	struct rgph_graph **pg, *g;
	int res, algo;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	algo = luaL_checkoption(L, 2, "", algo_strings);
	if (algo != 0) /* not default? */
		algo = *find_flag_by_name(algo_strings[algo]);

	g = *pg;
	res = rgph_assign(g, algo);

	switch (res) {
	case RGPH_SUCCESS:
		break;
	case RGPH_INVAL:
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid value");
		return 2;
	case RGPH_AGAIN:
		lua_pushboolean(L, 0);
		lua_pushstring(L, "try again");
		return 2;
	case RGPH_NOMEM:
		lua_pushboolean(L, 0);
		lua_pushstring(L, "not enough memory");
		return 2;
	default:
		lua_pushboolean(L, 0);
		lua_pushfstring(L, "unknown error %d", res);
		return 2;
	}

	/* Reuse pg for a different userdata. */
	pg = (struct rgph_graph **)lua_newuserdata(L, sizeof(pg));
	*pg = g;
	luaL_getmetatable(L, ASSIGN_MT);
	lua_setmetatable(L, -2);

	/* Keep a reference to the parent object. */
	lua_pushvalue(L, 1);
	setuservalue(L, -2);

	return 1;
}

static int
assign_get(lua_State *L)
{
	struct rgph_graph **pg;
	unsigned int val;
	int res;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, ASSIGN_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	res = rgph_copy_assignment(*pg, luaL_checkinteger(L, 2), &val);

	switch (res) {
	case RGPH_SUCCESS:
		lua_pushinteger(L, val);
		return 1;
	case RGPH_INVAL:
		return luaL_argerror(L, 1, "unassigned");
	case RGPH_RANGE:
		return luaL_argerror(L, 2, "out of range");
	default:
		return luaL_error(L, "unknown error %d", res);
	}
}

/* "peel,key" => EDGES_PEEL + (EDGES_KEY<<EDGES_SHIFT). */
static int
parse_edges_arg(lua_State *L, int n)
{
	char buf[64]; /* "index," x 10, rounded up */
	char *tok, *last_tok;
	const char *sep = ",";
	const char *opts;
	size_t optslen = 0;
	int shift = 0, res = 0;

	opts = lua_tolstring(L, n, &optslen);
	if (opts == NULL || optslen > sizeof(buf) - 1) {
		luaL_argerror(L, n, opts ? "too long" : "string expected");
		return res; /* UNREACHABLE */
	}

	memcpy(buf, opts, optslen);
	buf[optslen] = '\0';

	for (tok = strtok_r(buf, sep, &last_tok); tok != NULL;
	    tok = strtok_r(NULL, sep, &last_tok), shift += EDGES_SHIFT) {
		if (shift > 32 - EDGES_SHIFT) {
			luaL_argerror(L, n, "too many flags");
			return res; /* UNREACHABLE */
		} else if (strcmp(tok, "peel") == 0) {
			res |= EDGES_PEEL << shift;
		} else if (strcmp(tok, "key") == 0) {
			res |= EDGES_KEY << shift;
		} else if (strcmp(tok, "index") == 0) {
			res |= EDGES_INDEX << shift;
		} else {
			luaL_argerror(L, n, tok);
			return res; /* UNREACHABLE */
		}
	}

	return res;
}

static int
graph_edges_iter(lua_State *L)
{
	unsigned long edge[3];
	struct rgph_graph **pg;
	size_t at, peel = 0;
	int flags, nflags, res, rank;
	int key = 0, index = 0; /* Where key and index are on the Lua stack. */
	int iter, state, var; /* Indices of upvalues. */

	flags = lua_tointeger(L, lua_upvalueindex(2));
	at = lua_tointeger(L, lua_upvalueindex(3));

	pg = (struct rgph_graph **)lua_touserdata(L, lua_upvalueindex(1));
	if (*pg == NULL)
		return luaL_error(L, "dead %s object", GRAPH_MT);

	res = rgph_copy_edge(*pg, at, edge,
	    (flags & EDGES_PEEL_MASK) ? &peel : NULL);

	switch (res) {
	case RGPH_RANGE: return 0; /* Stop iterations.     */
	case RGPH_SUCCESS: break;  /* Push the edge later. */
	default: return luaL_error(L, "invalid value");
	}

	/* Save the next "at" value for the next iteration. */
	lua_pushinteger(L, at + 1);
	lua_replace(L, lua_upvalueindex(3));

	if (flags & EDGES_ITER_MASK) {
		iter = lua_upvalueindex(4);
		state = lua_upvalueindex(5);
		var = lua_upvalueindex(6);
		lua_pushvalue(L, iter);
		lua_pushvalue(L, state);
		lua_pushvalue(L, var);
		lua_call(L, 2, 3); /* Return key, data and index. */
		index = lua_gettop(L);
		key = index - 2;
		lua_pushvalue(L, -3); /* Save key for the next iteration.*/
		lua_replace(L, var);
	}

	/*
	 * Three elements are already pushed (if iter is required),
	 * this loop can push up to 10 elements, and 2 or 3 hashes
	 * will be pushed after the loop. It's less than LUA_MINSTACK.
	 */
	for (nflags = 0;
	    flags && nflags < 32 / EDGES_SHIFT;
	    flags >>= EDGES_SHIFT, nflags++) {
		if (flags & EDGES_PEEL)
			lua_pushinteger(L, peel);
		else if (flags & EDGES_KEY)
			lua_pushvalue(L, key);
		else if (flags & EDGES_INDEX)
			lua_pushvalue(L, index);
		else
			assert(0 && "parse_edges_arg() misbehaves");
	}

	rank = rgph_rank(*pg);
	switch (rank) {
	case 3:
		lua_pushinteger(L, edge[0]);
		/* FALLTHROUGH */
	case 2:
		lua_pushinteger(L, edge[rank - 2]);
		lua_pushinteger(L, edge[rank - 1]);
		return nflags + rank;
	default:
		return luaL_error(L, "invalid value");
	}
}

static int
graph_edges(lua_State *L)
{
	struct rgph_graph **pg;
	int flags, flstr;
	int nup; /* Number of upvalues. */

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	flstr = lua_isstring(L, 2) ? 1 : 0;
	flags = flstr ? parse_edges_arg(L, 2) : 0;

	nup = 3;
	lua_pushvalue(L, 1);
	lua_pushinteger(L, flags);
	lua_pushinteger(L, 0);

	if (flags & EDGES_ITER_MASK) {
		if (lua_isnoneornil(L, 2 + flstr)) {
			return luaL_argerror(L, 2 + flstr,
			    "iterator expected");
		}

		nup += 3;
		lua_pushvalue(L, 2 + flstr);

		if (lua_isnone(L, 3 + flstr))
			lua_pushnil(L);
		else
			lua_pushvalue(L, 3 + flstr);

		if (lua_isnone(L, 4 + flstr))
			lua_pushnil(L);
		else
			lua_pushvalue(L, 4 + flstr);
	} else {
		nup += 1;
		lua_pushnil(L);
	}

	lua_pushcclosure(L, &graph_edges_iter, nup);
	return 1;
}

static int
graph_edge(lua_State *L)
{
	unsigned long edge[3];
	struct rgph_graph **pg;
	int rank, res;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	res = rgph_copy_edge(*pg, luaL_checkinteger(L, 2) - 1, edge, NULL);

	switch (res) {
	case RGPH_RANGE:
		return luaL_error(L, "edge is out of range");
	case RGPH_SUCCESS:
		rank = rgph_rank(*pg);
		switch (rank) {
		case 3:
			lua_pushinteger(L, edge[0]);
			/* FALLTHROUGH */
		case 2:
			lua_pushinteger(L, edge[rank - 2]);
			lua_pushinteger(L, edge[rank - 1]);
			return rank;
		}
		/* FALLTHROUGH */
	case RGPH_INVAL:
	default:
		return luaL_error(L, "invalid value");
	}
}

static const luaL_Reg rgph_fn[] = {
	{ "new_graph", new_graph_fn },
	{ "count_keys", count_keys_fn },
	{ "fastdiv_prepare", fastdiv_prepare_fn },
	{ "jenkins2v", jenkins2v_fn },
	{ "murmur32v", murmur32v_fn },
	{ "murmur32s", murmur32s_fn },
	{ "xxh32s", xxh32s_fn },
	{ "xxh64s", xxh64s_fn },
	{ NULL, NULL }
};

static const luaL_Reg graph_fn[] = {
	{ "rank", graph_rank },
	{ "algo", graph_algo },
	{ "hash", graph_hash },
	{ "hash_bits", graph_hash_bits },
	{ "entries", graph_entries },
	{ "vertices", graph_vertices },
	{ "datalen_min", graph_datalen_min },
	{ "datalen_max", graph_datalen_max },
	{ "core_size", graph_core_size },
	{ "flags", graph_flags },
	{ "division_hint", graph_div_hint },
	{ "build", graph_build },
	{ "find_duplicates", graph_find_duplicates },
	{ "assign", graph_assign },
	{ "seed", graph_seed },
	{ "edge", graph_edge },
	{ "edges", graph_edges },
	{ NULL, NULL }
};

static const luaL_Reg graph_mt[] = {
	{ "__gc", graph_gc },
	{ NULL, NULL }
};

static const luaL_Reg assign_index[] = {
	{ "__index", assign_get },
	{ NULL, NULL }
};

static void
register_udata(lua_State *L, int arg, const char *tname,
    const luaL_Reg *metafunctions, const luaL_Reg *methods)
{

	/* Copy rgph module to the top. */
	if (arg != -1)
		lua_pushvalue(L, arg);

	/* Register methods as module's functions, e.g. rgph.build(). */
	if (methods != NULL) {
#if LUA_VERSION_NUM <= 501
		luaL_register(L, NULL, methods);
#else
		luaL_setfuncs(L, methods, 0);
#endif
	}

	luaL_newmetatable(L, tname);

	if (metafunctions != NULL) {
#if LUA_VERSION_NUM <= 501
		luaL_register(L, NULL, metafunctions);
#else
		luaL_setfuncs(L, metafunctions, 0);
#endif
	}

	/* Register methods, e.g. graph:build(). */
	if (methods != NULL) {
		lua_pushstring(L, "__index");
		lua_newtable(L);
#if LUA_VERSION_NUM <= 501
		luaL_register(L, NULL, methods);
#else
		luaL_setfuncs(L, methods, 0);
#endif
		lua_rawset(L, -3);
	}

	lua_pop(L, arg != -1 ? 2 : 1);
}

static void
push_jenkins2v_constants(lua_State *L)
{

	lua_newtable(L);
	lua_pushinteger(L, RGPH_JENKINS2V_SEED1);
	lua_setfield(L, -2, "seed1");
	lua_pushinteger(L, RGPH_JENKINS2V_SEED2);
	lua_setfield(L, -2, "seed2");
}

static void
push_murmur32v_constants(lua_State *L)
{

	lua_newtable(L);
	lua_pushinteger(L, RGPH_MURMUR32V_MUL1);
	lua_setfield(L, -2, "mul1");
	lua_pushinteger(L, RGPH_MURMUR32V_MUL2);
	lua_setfield(L, -2, "mul2");
	lua_pushinteger(L, RGPH_MURMUR32V_MUL3);
	lua_setfield(L, -2, "mul3");
	lua_pushinteger(L, RGPH_MURMUR32V_MUL4);
	lua_setfield(L, -2, "mul4");
	lua_pushinteger(L, RGPH_MURMUR32V_ADD1);
	lua_setfield(L, -2, "add1");
	lua_pushinteger(L, RGPH_MURMUR32V_ADD2);
	lua_setfield(L, -2, "add2");
	lua_pushinteger(L, RGPH_MURMUR32V_ADD3);
	lua_setfield(L, -2, "add3");
	lua_pushinteger(L, RGPH_MURMUR32V_ADD4);
	lua_setfield(L, -2, "add4");
	lua_pushinteger(L, RGPH_MURMUR32V_FMIXMUL1);
	lua_setfield(L, -2, "fmixmul1");
	lua_pushinteger(L, RGPH_MURMUR32V_FMIXMUL2);
	lua_setfield(L, -2, "fmixmul2");
}

static void
push_murmur32s_constants(lua_State *L)
{

	lua_newtable(L);
	lua_pushinteger(L, RGPH_MURMUR32S_MUL1);
	lua_setfield(L, -2, "mul1");
	lua_pushinteger(L, RGPH_MURMUR32S_MUL2);
	lua_setfield(L, -2, "mul2");
	lua_pushinteger(L, RGPH_MURMUR32S_ADD1);
	lua_setfield(L, -2, "add1");
	lua_pushinteger(L, RGPH_MURMUR32S_FMIXMUL1);
	lua_setfield(L, -2, "fmixmul1");
	lua_pushinteger(L, RGPH_MURMUR32S_FMIXMUL2);
	lua_setfield(L, -2, "fmixmul2");
}

static void
push_xxh32s_constants(lua_State *L)
{

	lua_newtable(L);
	lua_pushinteger(L, RGPH_XXH32S_PRIME1);
	lua_setfield(L, -2, "prime1");
	lua_pushinteger(L, RGPH_XXH32S_PRIME2);
	lua_setfield(L, -2, "prime2");
	lua_pushinteger(L, RGPH_XXH32S_PRIME3);
	lua_setfield(L, -2, "prime3");
	lua_pushinteger(L, RGPH_XXH32S_PRIME4);
	lua_setfield(L, -2, "prime4");
	lua_pushinteger(L, RGPH_XXH32S_PRIME5);
	lua_setfield(L, -2, "prime5");
}

static void
push_xxh64s_constants(lua_State *L)
{

	lua_newtable(L);

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, RGPH_XXH64S_PRIME1 & 0xffffffffu);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, RGPH_XXH64S_PRIME1 >> 32);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "prime1");

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, RGPH_XXH64S_PRIME2 & 0xffffffffu);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, RGPH_XXH64S_PRIME2 >> 32);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "prime2");

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, RGPH_XXH64S_PRIME3 & 0xffffffffu);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, RGPH_XXH64S_PRIME3 >> 32);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "prime3");

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, RGPH_XXH64S_PRIME4 & 0xffffffffu);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, RGPH_XXH64S_PRIME4 >> 32);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "prime4");

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, RGPH_XXH64S_PRIME5 & 0xffffffffu);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, RGPH_XXH64S_PRIME5 >> 32);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "prime5");
}

int
luaopen_rgph(lua_State *L)
{

#if LUA_VERSION_NUM <= 501
	luaL_register(L, "rgph", rgph_fn);
#else
	luaL_newlib(L, rgph_fn);
#endif

	register_udata(L, -1, GRAPH_MT, graph_mt, graph_fn);

	luaL_newmetatable(L, ASSIGN_MT);
#if LUA_VERSION_NUM <= 501
	luaL_register(L, NULL, assign_index);
#else
	luaL_setfuncs(L, assign_index, 0);
#endif
	lua_pop(L, 1);

	lua_newtable(L); /* rgph.const */
	push_jenkins2v_constants(L);
	lua_setfield(L, -2, "jenkins2v");
	push_murmur32v_constants(L);
	lua_setfield(L, -2, "murmur32v");
	push_murmur32s_constants(L);
	lua_setfield(L, -2, "murmur32s");
	push_xxh32s_constants(L);
	lua_setfield(L, -2, "xxh32s");
	push_xxh64s_constants(L);
	lua_setfield(L, -2, "xxh64s");

	lua_setfield(L, -2, "const");

	return 1;
}
