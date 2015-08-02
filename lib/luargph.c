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

#include <errno.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "rgph.h"

#define GRAPH_MT "rgph.graph"

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

	/* XXX size_t vs lua_Integer */
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

	/* XXX size_t vs lua_Integer */
	lua_pushinteger(L, rgph_vertices(*pg));
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
