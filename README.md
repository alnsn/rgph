# rgph
Random Graph Perfect Hashing with optional Lua bindings

## Building

Shared library:
	cd lib && make CFLAGS='-O2 -g' CXXFLAGS='-O2 -g' librgph.so

To build Lua module, you need pkg-config Lua package. XXX luarocks.
	cd lib && make CFLAGS='-O2 -g' CXXFLAGS='-O2 -g' rgph.so
