OBJ=	writer.o jenkins2.o
ALL=	librgph.so # XXX .dylib on MacOS

# Comment these lines to debug.
CFLAGS+=	-DNDEBUG
CXXFLAGS+=	-DNDEBUG

WARNS?=		-Wall
C99OPTS?=	-std=c99
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti
CFLAGS+=	-O2 -fPIC ${WARNS} -I. ${C99OPTS}
CXXFLAGS+=	-O2 -fPIC ${WARNS} -I. ${CXXOPTS}
SHLDFLAGS=	-O2 -fPIC ${PIEOPTS} -shared

all: ${ALL}

librgph.so: ${OBJ}
	${CC} ${OBJ} ${SHLDFLAGS} -o librgph.so

clean:
	rm -f ${OBJ} ${ALL}
