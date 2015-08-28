.POSIX:

DSO?=		so  # or dylib
LUAPKG?=	lua # pkg-config name

WARNS?=		-Wall
PICFLAGS?=	-fPIC
PICLDFLAGS?=	-fPIC
C99OPTS?=	-std=c99
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti

# PACKAGE DEVELOPERS: don't uncomment the lines below, pass those flags
# to your make instead: make CFLAGS='-O2 -UNDEBUG' CXXFLAGS='-O2 -UNDEBUG'.
#CFLAGS+=	-DRGPH_UNALIGNED_READ
#CXXFLAGS+=	-DRGPH_UNALIGNED_READ
#CFLAGS+=	-UNDEBUG
#CXXFLAGS+=	-UNDEBUG


OBJ=		writer.o    jenkins2.o    murmur32.o    murmur32s.o
PICOBJ=		writer.pico jenkins2.pico murmur32.pico murmur32s.pico
LOBJ=		luargph.o
LPICOBJ=	luargph.pico

XCFLAGS+=	-I. -DNDEBUG $(WARNS) $(C99OPTS)
XCXXFLAGS+=	-I. -DNDEBUG $(WARNS) $(CXXOPTS)


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

rgph.$(DSO): $(PICOBJ) $(LPICOBJ)
	$(CC) `pkg-config --cflags --libs $(LUAPKG)` $(PICLDFLAGS) $(LDFLAGS) -shared $(PICOBJ) $(LPICOBJ) -o $@

clean:
	rm -f $(OBJ) $(PICOBJ) $(LOBJ) $(LPICOBJ) *.so *.dylib *.a
