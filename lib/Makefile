.POSIX:

DSO?=		so  # or dylib
LUAPKG?=	lua # pkg-config name

WARNS?=		-Wall
PICFLAGS?=	-fPIC
C99OPTS?=	-std=c99
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti

#CFLAGS+=	-DRGPH_UNALIGNED_READ


OBJ=		writer.o    jenkins2.o    murmur32.o    murmur32s.o
PICOBJ=		writer.pico jenkins2.pico murmur32.pico murmur32s.pico
LOBJ=		luargph.o
LPICOBJ=	luargph.pico

CFLAGS+=	-O2 $(WARNS) -I. $(C99OPTS)
CXXFLAGS+=	-O2 $(WARNS) -I. $(CXXOPTS)

# Comment these lines to debug.
CFLAGS+=	-DNDEBUG
CXXFLAGS+=	-DNDEBUG

.SUFFIXES: .c .cc .o .pico

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cc.pico:
	$(CXX) $(CXXFLAGS) $(PICFLAGS) -c $< -o $@

.c.pico:
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

all: librgph.so

luargph.pico: luargph.c
	$(CC) $(CFLAGS) $(PICFLAGS) `pkg-config --cflags --cflags $(LUAPKG)` -c $< -o $@

librgph.$(DSO): $(PICOBJ)
	$(CC) $(LDFLAGS) -shared $(PICOBJ) -o $@

rgph.$(DSO): $(PICOBJ) $(LPICOBJ)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` -shared $(PICOBJ) $(LPICOBJ) -o $@

clean:
	rm -f $(OBJ) $(PICOBJ) $(LOBJ) $(LPICOBJ) *.so *.dylib *.a
