/*-
 * Copyright (c) 2016 Alexander Nasonov.
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

#include <stdint.h>

#include <lua.h>
#include <lauxlib.h>

#include "rgph_hash.h"

static int
jenkins2(lua_State *L)
{
	uint32_t h[3], seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	rgph_u32x3_jenkins2_data(str, len, seed, h);

	lua_createtable(L, 3, 0);
	lua_pushinteger(L, h[0]);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, h[1]);
	lua_rawseti(L, -2, 2);
	lua_pushinteger(L, h[2]);
	lua_rawseti(L, -2, 3);

	return 1;
}

static int
murmur32(lua_State *L)
{
	uint32_t h[4], seed;
	const char *str;
	size_t len;

	str = luaL_checklstring(L, 1, &len);
	seed = luaL_checkinteger(L, 2);

	rgph_u32x4_murmur32_data(str, len, seed, h);

	lua_createtable(L, 4, 0);
	lua_pushinteger(L, h[0]);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, h[1]);
	lua_rawseti(L, -2, 2);
	lua_pushinteger(L, h[2]);
	lua_rawseti(L, -2, 3);
	lua_pushinteger(L, h[3]);
	lua_rawseti(L, -2, 4);

	return 1;
}

static const luaL_Reg hash_fn[] = {
	{ "jenkins2",  jenkins2  },
	/* NIY { "jenkins3",  jenkins3  }, */
	/* NIY { "jenkins3s", jenkins3s }, */
	{ "murmur32",  murmur32  },
	/* NIY { "murmur32s", murmur32s }, */
	/* NIY { "xxh32s",    xxh32s    }, */
	/* NIY { "xxh64s",    xxh64s    }, */
	{ NULL, NULL }
};

int
luaopen_rgph_hash(lua_State *L)
{

#if LUA_VERSION_NUM <= 501
	luaL_register(L, "rgph.hash", hash_fn);
#else
	luaL_newlib(L, hash_fn);
#endif

	return 1;
}
