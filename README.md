# rgph
Random Graph Perfect Hashing with optional Lua bindings

## Building

You need `c99`, `c++` compilers and POSIX-compatible `make` utility
to build the library and the tests.  Makefiles don't use any fancy
stuff and there is no support for auto configuration.

Typing `(cd lib && make)` should work but it may produce unoptimised
binaries. It's best to set optimisation flags in `CFLAGS` and
`CXXFLAGS` variables. You may also want to pass `-DUNALIGNED_READ`
when building on platforms with fast unaligned reads.

To build a shared library:

    (cd lib && make CFLAGS='-O2 -g' CXXFLAGS='-O2 -g' all-c)

To build on OSX, you should pass `DSO=dylib`:

    (cd lib && make CFLAGS='-O2 -g' CXXFLAGS='-O2 -g' DSO=dylib all-c)

To build the Lua module, you need `pkg-config` Lua package. XXX luarocks.

    (cd lib && make CFLAGS='-O2 -g' CXXFLAGS='-O2 -g' all-lua)

The above command assumes that your Lua package is known to `pkg-config`
as `lua`. You can specify an alternative name with `LUAPKG=lua-5.3`.

Lua versions `5.1`, `5.2` and `5.3` are supported.
