.POSIX:

DSO?=		so  # or dylib
LUAPKG?=	lua # pkg-config name

WARNS?=		-Wall -Wextra
PICFLAGS?=	-fPIC
PICLDFLAGS?=	-fPIC
C99OPTS?=	-std=gnu99 # GNU for WEAK_ALIASES
CXXOPTS?=	-std=c++11 -nostdinc++ -fno-exceptions -fno-rtti

CPPFLAGS+=	-DNDEBUG -DWEAK_ALIASES

# PACKAGE DEVELOPERS: don't uncomment the lines below, pass those flags
# to your make instead: make CFLAGS='-O2 -UNDEBUG' CXXFLAGS='-O2 -UNDEBUG'.
#CFLAGS+=	-DUNALIGNED_READ
#CXXFLAGS+=	-DUNALIGNED_READ
#CFLAGS+=	-UNDEBUG
#CXXFLAGS+=	-UNDEBUG
#CFLAGS+=	-UWEAK_ALIASES

XCFLAGS=	-I. $(CPPFLAGS) $(WARNS) $(C99OPTS)
XCXXFLAGS=	-I. $(CPPFLAGS) $(WARNS) $(CXXOPTS)

GRAPHO=		graph.o    fastdiv.o
GRAPHPICO=	graph.pico fastdiv.pico

HASHO=		jenkins2v.o    murmur32v.o    murmur32s.o    xxh32s.o    xxh64s.o
HASHPICO=	jenkins2v.pico murmur32v.pico murmur32s.pico xxh32s.pico xxh64s.pico

HASHEXO=	jenkins2v-ex.o    murmur32v-ex.o    murmur32s-ex.o    xxh32s-ex.o    xxh64s-ex.o
HASHEXPICO=	jenkins2v-ex.pico murmur32v-ex.pico murmur32s-ex.pico xxh32s-ex.pico xxh64s-ex.pico

OBJ=		$(GRAPHO)    $(HASHO)
PICO=		$(GRAPHPICO) $(HASHPICO)

LUARGPHPICO=	luargph.pico $(PICO)
LUAHASHPICO=	luahash.pico $(HASHPICO) $(HASHEXPICO)


.SUFFIXES: .c .cc .o .pico

.cc.o:
	$(CXX) $(XCXXFLAGS) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(XCFLAGS) $(CFLAGS)  -c $< -o $@

.cc.pico:
	$(CXX) $(XCXXFLAGS) $(PICFLAGS) $(CXXFLAGS) -c $< -o $@

.c.pico:
	$(CC) $(XCFLAGS) $(PICFLAGS) $(CXXFLAGS) -c $< -o $@

all-c: librgph.$(DSO) librgph_hash.$(DSO)
all-lua: rgph.$(DSO) #hash.$(DSO)
all: all-c all-lua

luargph.pico: luargph.c
	$(CC) `pkg-config --cflags $(LUAPKG)` $(XCFLAGS) $(PICFLAGS) $(CFLAGS) -c $< -o $@

librgph.$(DSO): $(PICO)
	$(CC) $(PICLDFLAGS) $(LDFLAGS) -shared $(PICO) -o $@

librgph_hash.$(DSO): $(HASHEXPICO)
	$(CC) $(PICLDFLAGS) $(LDFLAGS) -shared $(HASHEXPICO) -o $@

rgph.$(DSO): $(LUARGPHPICO)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` $(PICLDFLAGS) $(LDFLAGS) -shared $(LUARGPHPICO) -o $@

hash.$(DSO): $(LUAHASHPICO)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` $(PICLDFLAGS) $(LDFLAGS) -shared $(LUAHASHPICO) -o $@

clean:
	rm -f $(OBJ) $(PICO) $(LUARGPHPICO) $(LUAHASHPICO) *.so *.dylib *.a
