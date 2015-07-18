SRC=	lib/writer.C
TESTS=	tests/t_rgph.c
ALL=	librgph.so

CFLAGS+=	-Wall -DNDEBUG
SHLDFLAGS=	-fPIC -shared

all: ${ALL}

librgph.so: ${SRC}
	${CC} ${CFLAGS} ${SHLDFLAGS} ${SRC} -o librgph.so

t_rgph: ${TESTS}
	${CC} ${CFLAGS} -fPIC -Ilib ${TESTS} -L. -lrgph -o t_rgph

clean:
	rm -f ${ALL} test.rgph
