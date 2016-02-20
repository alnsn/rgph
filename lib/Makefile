.POSIX:

DSO?=		so  # or dylib
LUAPKG?=	lua # pkg-config name

WARNS?=		-Wall -Wextra
PICFLAGS?=	-fPIC
PICLDFLAGS?=	-fPIC
C99OPTS?=	-std=gnu99 # GNU for WEAK_ALIASES
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti

CPPFLAGS+=	-DNDEBUG -DWEAK_ALIASES

# PACKAGE DEVELOPERS: don't uncomment the lines below, pass those flags
# to your make instead: make CFLAGS='-O2 -UNDEBUG' CXXFLAGS='-O2 -UNDEBUG'.
#CFLAGS+=	-DUNALIGNED_READ
#CXXFLAGS+=	-DUNALIGNED_READ
#CFLAGS+=	-UNDEBUG
#CXXFLAGS+=	-UNDEBUG
#CFLAGS+=	-UWEAK_ALIASES


OBJ=		graph.o    jenkins2.o    murmur32.o    murmur32s.o    xxh32s.o    xxh64s.o
PICOBJ=		graph.pico jenkins2.pico murmur32.pico murmur32s.pico xxh32s.pico xxh64s.pico
LRGPHOBJ=	luargph.pico
LHASHOBJ=	luahash.pico

XCFLAGS=	-I. $(CPPFLAGS) $(WARNS) $(C99OPTS)
XCXXFLAGS=	-I. $(CPPFLAGS) $(WARNS) $(CXXOPTS)


.SUFFIXES: .c .cc .o .pico

.cc.o:
	$(CXX) $(XCXXFLAGS) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(XCFLAGS) $(CFLAGS)  -c $< -o $@

.cc.pico:
	$(CXX) $(XCXXFLAGS) $(PICFLAGS) $(CXXFLAGS) -c $< -o $@

.c.pico:
	$(CC) $(XCFLAGS) $(PICFLAGS) $(CXXFLAGS) -c $< -o $@

all: librgph.so

luargph.pico: luargph.c
	$(CC) `pkg-config --cflags --cflags $(LUAPKG)` $(XCFLAGS) $(PICFLAGS) $(CFLAGS) -c $< -o $@

librgph.$(DSO): $(PICOBJ)
	$(CC) $(PICLDFLAGS) $(LDFLAGS) -shared $(PICOBJ) -o $@

rgph.$(DSO): $(PICOBJ) $(LRGPHOBJ)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` $(PICLDFLAGS) $(LDFLAGS) -shared $(PICOBJ) $(LRGPHOBJ) -o $@

hash.$(DSO): $(PICOBJ) $(LHASHOBJ)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` $(PICLDFLAGS) $(LDFLAGS) -shared $(PICOBJ) $(LHASHOBJ) -o $@

clean:
	rm -f $(OBJ) $(PICOBJ) $(LRGPHOBJ) $(LHASHOBJ) *.so *.dylib *.a
