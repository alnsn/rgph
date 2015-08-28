.POSIX:

OBJ=	t_main.o t_jenkins2.o t_murmur32.o t_murmur32s.o

WARNS?=		-Wall
C99OPTS?=	-std=c99
CFLAGS+=	-O2 -fPIC ${WARNS} -I. -I../lib ${C99OPTS}
LDFLAGS+=	-O2 -fPIC

all: t_rgph

t_rgph: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -L../lib -lrgph -o t_rgph

clean:
	rm -f *.o t_rgph
