/*
 * (cd ../lib && make all-c)
 * afl-g++ -std=gnu++11 -fPIC -O -I../lib fuzz.c ../lib/graph.cc ../lib/murmur32s.pico ../lib/murmur32v.pico ../lib/xxh32s.pico ../lib/xxh64s.pico ../lib/fastdiv.pico ../lib/jenkins2v.pico
 *
 * ./a.out && mv fuzz.dat ${INPUTDIR:?}
 * afl-fuzz -i ${INPUTDIR:?} -o ${OUTPUTDIR:?} ./a.out @@
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "rgph_defs.h"
#include "rgph_graph.h"

static const char letters[] = "abcdefghijklmnopqrstuvwxyz";

struct fuzz_entry {
	uint64_t index;
	uint8_t has_index;
	uint8_t keylen;
	char key[54];
};

struct fuzz_header {
	size_t nverts;
	unsigned long seed;
	int alloc_flags;
	int build_flags;
	int assign_flags;
	unsigned int nentries;
	struct fuzz_entry entries[];
};

struct iterator_state {
	size_t pos;
	size_t nentries;
	struct fuzz_entry *entries;
	struct rgph_entry out;
};

static inline size_t
min(size_t a, size_t b)
{

	return a < b ? a : b;
}

static inline size_t
max(size_t a, size_t b)
{

	return a > b ? a : b;
}

static const struct rgph_entry *
iterator_func(void *state)
{
	struct iterator_state *s = (struct iterator_state *)state;
	struct fuzz_entry *in = &s->entries[s->pos];
	struct rgph_entry *out = &s->out;

	if (s->pos == s->nentries)
		return NULL;

	out->key = out->data = in->key;
	out->keylen = out->datalen = min(sizeof(in->key), in->keylen);
	out->index = in->index;
	out->has_index = in->has_index;

	s->pos++;
	return out;
}

int
write_sample_input(void)
{
	fuzz_header h;
	FILE *f;
	size_t i;

	f = fopen("fuzz.dat", "w");
	if (f == NULL)
		return EXIT_FAILURE;

	h.nverts = h.nentries = sizeof(letters) - 1;
	h.seed = 123456789;
	h.alloc_flags = 0;
	h.build_flags = 0;
	h.assign_flags = 0;

	if (fwrite(&h, 1, sizeof(h), f) != sizeof(h)) {
		fclose(f);
		return EXIT_FAILURE;
	}

	for (i = 0; i < sizeof(letters) - 1; i++) {
		struct fuzz_entry e;

		e.index = i;
		e.has_index = true;
		e.keylen = 2;
		memcpy(e.key, &letters[i], 2);

		if (fwrite(&e, 1, sizeof(e), f) != sizeof(e)) {
			fclose(f);
			return EXIT_FAILURE;
		}
	}

	fclose(f);
	return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
	iterator_state state;
	struct stat st;
	struct fuzz_header *h;
	struct rgph_graph *g;
	void *buf;
	size_t flen;
	unsigned long seed;
	int fd, res;

	if (argc < 2)
		return write_sample_input();

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return EXIT_FAILURE;

	if (fstat(fd, &st) == -1) {
		close(fd);
		return EXIT_FAILURE;
	}

	flen = max(st.st_size, sizeof(*h));

	buf = mmap(NULL, flen, PROT_READ, MAP_FILE, fd, 0);
	h = (struct fuzz_header *)buf;
	close(fd);

	if (buf == MAP_FAILED)
		return EXIT_FAILURE;

	g = rgph_alloc_graph(h->nverts, h->alloc_flags);
	if (g == NULL)
		return EXIT_FAILURE;

	seed = h->seed;
	state.entries = h->entries;

	while (true) {
		size_t dup;

		state.pos = 0;
		state.nentries = min(h->nentries,
		    (flen - sizeof(*h)) / sizeof(h->entries[0]));

		res = rgph_build_graph(g, h->build_flags, seed++,
		    &iterator_func, &state);
		if (res == RGPH_SUCCESS || res != RGPH_AGAIN)
			break;

		state.pos = 0;
		res = rgph_find_duplicates(g, &iterator_func, &state, &dup);
		if (res == RGPH_SUCCESS)
			break;
	}

	res = rgph_assign(g, h->assign_flags);

	rgph_free_graph(g);

	return res == RGPH_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
