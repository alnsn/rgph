OBJ=	lib/writer.o lib/jenkins2.o
TESTS=	tests/t_rgph.c
ALL=	librgph.so # XXX .dylib on MacOS

# Comment these lines to debug.
CFLAGS+=	-DNDEBUG
CXXFLAGS+=	-DNDEBUG

WARNS?=		-Wall
PIEOPTS?=	-fPIE
C99OPTS?=	-std=c99
CXXOPTS?=	-nostdinc++ -fno-exceptions -fno-rtti
CFLAGS+=	-O2 -fPIC ${PIEOPTS} ${WARNS} -Ilib ${C99OPTS}
CXXFLAGS+=	-O2 -fPIC ${PIEOPTS} ${WARNS} -Ilib ${CXXOPTS}
SHLDFLAGS=	-O2 -fPIC ${PIEOPTS} -shared

all: ${ALL}

librgph.so: ${OBJ}
	${CC} ${OBJ} ${SHLDFLAGS} -o librgph.so

t_rgph: ${TESTS}
	${CC} ${CFLAGS} -fPIC -Ilib ${TESTS} -L. -lrgph -o t_rgph

clean:
	rm -f ${OBJ} ${ALL}
