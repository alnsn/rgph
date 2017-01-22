.POSIX:

OBJ=	t_main.o t_fastdiv.o t_jenkins2v.o t_murmur32v.o t_murmur32s.o t_t1ha64s.o t_xxh32s.o t_xxh64s.o

WARNS?=		-Wall -Wextra
C99OPTS?=	-std=c99

XCFLAGS=	$(WARNS) -I. -I../lib $(C99OPTS)
XLDFLAGS=	-L../lib -lrgph -lrgph_hash

all: t_rgph

.c.o:
	$(CC) $(XCFLAGS) $(CFLAGS)  -c $< -o $@

t_rgph: $(OBJ)
	$(CC) $(OBJ) $(XLDFLAGS) $(LDFLAGS) -o t_rgph

clean:
	rm -f *.o t_rgph
