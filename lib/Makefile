OBJ=	writer.o jenkins2.o murmur32.o
LOBJ=	luargph.o

#CFLAGS+=	-DRGPH_UNALIGNED_READ

# Comment these lines to debug.
CFLAGS+=	-DNDEBUG
CXXFLAGS+=	-DNDEBUG

WARNS?=		-Wall
C99OPTS?=	-std=c99
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti
CFLAGS+=	-O2 -fPIC ${WARNS} -I. ${C99OPTS}
CXXFLAGS+=	-O2 -fPIC ${WARNS} -I. ${CXXOPTS}
SHLDFLAGS=	-O2 -fPIC -shared
LDFLAGS+=	${SHLDFLAGS}

all: librgph.so # XXX .dylib on MacOS

librgph.so: ${OBJ}
	${CC} ${OBJ} ${LDFLAGS} -o librgph.so

lua: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.1.so

lua51: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua51` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.1.so

lua52: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua52` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.2.so

lua53: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua53` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.3.so

lua5.1: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua5.1` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.1.so

lua5.2: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua5.2` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.2.so

lua5.3: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua5.3` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.3.so

lua-5.1: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua-5.1` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.1.so

lua-5.2: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua-5.2` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.2.so

lua-5.3: ${OBJ} ${LOBJ}
	${CC} `pkg-config --cflags --libs lua-5.3` ${LDFLAGS} ${OBJ} ${LOBJ} -o librgph5.3.so

clean:
	rm -f ${OBJ} ${LOBJ} librgph*.so
