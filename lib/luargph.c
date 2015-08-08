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
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "rgph.h"

#define GRAPH_MT "rgph.graph"

struct build_iter_state {
	struct rgph_entry ent;
	lua_State *L;
	int iter;  /* Index of an iterator function. */
};

static int
parse_flags(lua_State *L, int arg)
{
	char buf[64];
	char *tok, *last_tok;
	const char *sep = "+|,";
	const char *opts;
	size_t optslen = 0;
	int flags = 0; /* aka RGPH_DEFAULT */

	opts = luaL_optlstring(L, arg, "", &optslen);
	if (optslen > sizeof(buf) - 1)
		return luaL_argerror(L, arg, "too long");

	memcpy(buf, opts, optslen);
	buf[optslen] = '\0';

	for (tok = strtok_r(buf, sep, &last_tok); tok != NULL;
	    tok = strtok_r(NULL, sep, &last_tok)) {
		if (strcmp(tok, "rank2") == 0)
			flags |= RGPH_RANK2;
		else if (strcmp(tok, "rank3") == 0)
			flags |= RGPH_RANK3;
		else if (strcmp(tok, "chm") == 0)
			flags |= RGPH_ALGO_CHM;
		else if (strcmp(tok, "bdz") == 0)
			flags |= RGPH_ALGO_BDZ;
		else if (strcmp(tok, "jenkins2") == 0)
			flags |= RGPH_HASH_JENKINS2;
		else if (strcmp(tok, "jenkins3") == 0)
			flags |= RGPH_HASH_JENKINS3;
		else if (strcmp(tok, "murmur3") == 0)
			flags |= RGPH_HASH_MURMUR3;
		else
			return luaL_argerror(L, arg, "parse error");
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
			return luaL_argerror(L, 1, "not in range");
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
graph_entries(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushnumber(L, rgph_entries(*pg));
	return 1;
}

static int
graph_vertices(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushnumber(L, rgph_vertices(*pg));
	return 1;
}

static int
graph_seed(lua_State *L)
{
	struct rgph_graph **pg;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	lua_pushnumber(L, rgph_seed(*pg));
	return 1;
}

static struct rgph_entry *
graph_build_iter(void *raw_state)
{
	struct build_iter_state *state = (struct build_iter_state *)raw_state;
	lua_State *L = state->L;

	lua_pushvalue(L, state->iter);     /* Push the iterator function. */
	lua_pushvalue(L, state->iter + 1); /* Push the state. */
	lua_pushvalue(L, state->iter + 2); /* Push the first var. */

	/* Call the iterator function and return 1 var; may throw Lua error. */
	lua_call(L, 2, 1);
	lua_replace(L, state->iter + 2); /* Move the first var to its place. */

	/* XXX don't convert to string. */
	state->ent.key = lua_tolstring(L, state->iter + 2, &state->ent.keylen);
	return state->ent.key == NULL ? NULL : &state->ent;
}

static int
graph_build(lua_State *L)
{
	struct build_iter_state state;
	struct rgph_graph **pg;
	unsigned long seed;
	int res;

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	seed = luaL_checkinteger(L, 2);

	state.iter = 3;
	luaL_checkany(L, state.iter); /* Will check later if it's callable. */

	/* Align stack as if state and var were always present. */
	lua_settop(L, state.iter + 2);

	memset(&state.ent, 0, sizeof(state.ent));
	state.L = L;

	res = rgph_build_graph(*pg, seed, &graph_build_iter, &state);

	lua_pushboolean(L, res == RGPH_SUCCESS);
	switch (res) {
	case RGPH_CYCLE:
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
graph_edges_iter(lua_State *L)
{
	unsigned long edge[3];
	struct rgph_graph **pg;
	size_t at;
	int key, rank, res;
	int iter, state, var; /* Indices of upvalues. */

	iter = lua_upvalueindex(3);
	key = !lua_isnil(L, iter);

	if (key) {
		state = lua_upvalueindex(4);
		var = lua_upvalueindex(5);
		lua_pushvalue(L, iter);
		lua_pushvalue(L, state);
		lua_pushvalue(L, var);
		lua_call(L, 2, 1);
		lua_pushvalue(L, -1);
		lua_replace(L, var);
	}

	at = lua_tointeger(L, lua_upvalueindex(2));
	lua_pushinteger(L, at + 1);
	lua_replace(L, lua_upvalueindex(2));

	pg = (struct rgph_graph **)lua_touserdata(L, lua_upvalueindex(1));
	res = rgph_copy_edge(*pg, at, edge);

	switch (res) {
	case RGPH_RANGE:
		return 0;
	case RGPH_SUCCESS:
		rank = rgph_rank(*pg);
		switch (rank) {
		case 3:
			lua_pushinteger(L, edge[0]);
			/* FALLTHROUGH */
		case 2:
			lua_pushinteger(L, edge[rank - 2]);
			lua_pushinteger(L, edge[rank - 1]);
			return key + rank;
		}
		/* FALLTHROUGH */
	case RGPH_INVAL:
	default:
		return luaL_error(L, "invalid value");
	}
}

static int
graph_edges(lua_State *L)
{
	struct rgph_graph **pg;
	int nup; /* Number of upvalues. */

	pg = (struct rgph_graph **)luaL_checkudata(L, 1, GRAPH_MT);
	if (*pg == NULL)
		return luaL_argerror(L, 1, "dead object");

	nup = 2;
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 0);

	if (lua_isnoneornil(L, 2)) {
		nup += 1;
		lua_pushnil(L);
	} else {
		nup += 3;
		lua_pushvalue(L, 2);

		if (lua_isnone(L, 3))
			lua_pushnil(L);
		else
			lua_pushvalue(L, 3);

		if (lua_isnone(L, 4))
			lua_pushnil(L);
		else
			lua_pushvalue(L, 4);
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

	res = rgph_copy_edge(*pg, luaL_checkinteger(L, 2) - 1, edge);

	switch (res) {
	case RGPH_RANGE:
		return luaL_error(L, "edge not in range");
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

static luaL_Reg rgph_fn[] = {
	{ "new_graph", new_graph_fn },
	{ NULL, NULL }
};

static luaL_Reg graph_fn[] = {
	{ "rank", graph_rank },
	{ "entries", graph_entries },
	{ "vertices", graph_vertices },
	{ "build", graph_build },
	{ "seed", graph_seed },
	{ "edge", graph_edge },
	{ "edges", graph_edges },
	{ "core_size", graph_core_size },
	{ NULL, NULL }
};

static luaL_Reg graph_mt[] = {
	{ "__gc", graph_gc },
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

int
luaopen_rgph(lua_State *L)
{

#if LUA_VERSION_NUM <= 501
	luaL_register(L, "rgph", rgph_fn);
#else
	luaL_newlib(L, rgph_fn);
#endif

	register_udata(L, -1, GRAPH_MT, graph_mt, graph_fn);

	return 1;
}
