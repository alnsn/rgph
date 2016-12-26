/*
 * (cd ../lib && make all-c)
 * afl-g++ -std=gnu++11 -fPIC -O -I../lib fuzz.cc ../lib/graph.cc ../lib/murmur32s.pico ../lib/murmur32v.pico ../lib/xxh32s.pico ../lib/xxh64s.pico ../lib/fastdiv.pico ../lib/jenkins2v.pico
 *
 * ./a.out && mv fuzz.dat ${INPUTDIR:?}
 * afl-fuzz -i ${INPUTDIR:?} -o ${OUTPUTDIR:?} ./a.out @@
 */

#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <set>
#include <string>

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
	size_t alloc_nkeys;
	size_t build_nkeys;
	unsigned long seed;
	int alloc_flags;
	int build_flags;
	int assign_flags;
	struct fuzz_entry entries[];
};

struct iterator_state {
	size_t pos;
	size_t alloc_nkeys;
	size_t build_nkeys;
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
	struct fuzz_entry *in = &s->entries[s->pos % s->nentries];
	struct rgph_entry *out = &s->out;

	if (s->pos == s->build_nkeys)
		return NULL;

	out->key = out->data = in->key;
	out->keylen = out->datalen = min(sizeof(in->key), in->keylen);
	out->index = in->index;
	out->has_index = in->has_index;

	s->pos++;
	return out;
}

static bool
has_duplicates(const struct fuzz_entry *entries, size_t nentries)
{
	std::set<std::string> keys;

	for (size_t i = 0; i < nentries; i++) {
		size_t keylen = min(entries[i].keylen,
		    sizeof(entries[i].key));

		if (!keys.insert(std::string(entries[i].key, keylen)).second)
			return true;
	}

	return false;
}

static void
check_bdz(struct rgph_graph *g)
{
	size_t const nkeys = rgph_entries(g);
	int const rank = (rgph_flags(g) & RGPH_RANK3) ? 3 : 2;
	std::set<uint32_t> hashes;
	size_t width = 0;
	int res;
	uint8_t const *assign;

	assign = (uint8_t const *)rgph_assignments(g, &width);
	assert(assign != nullptr && width == 1);

	for (size_t i = 0; i < nkeys; i++) {
		unsigned long edge[3];
		int h = 0;

		res = rgph_copy_edge(g, i, edge, nullptr);
		assert(res == RGPH_SUCCESS);

		for (int j = 0; j < rank; j++) {
			assert(edge[j] < nkeys && assign[edge[j]] < rank);
			h += assign[edge[j]];
			if (h >= rank)
				h -= rank;
		}

		bool inserted = hashes.insert(edge[h]).second;
		assert(inserted);
	}
}

int
write_sample_input(void)
{
	fuzz_header h;
	FILE *f;
	size_t i;

	memset(&h, 0, sizeof(h));

	f = fopen("fuzz.dat", "w");
	if (f == NULL)
		return EXIT_FAILURE;

	h.alloc_nkeys = h.build_nkeys = sizeof(letters) - 1;
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
	int build_flags, fd, res;
	bool dup_checked = false;

	if (argc < 2)
		return write_sample_input();

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return EXIT_FAILURE;

	if (fstat(fd, &st) == -1) {
		close(fd);
		return EXIT_FAILURE;
	}

	flen = max(st.st_size, sizeof(*h) + sizeof(h->entries[0]));

	buf = mmap(NULL, flen, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	h = (struct fuzz_header *)buf;
	close(fd);

	if (buf == MAP_FAILED)
		return EXIT_FAILURE;

	g = rgph_alloc_graph(h->alloc_nkeys, h->alloc_flags);
	if (g == NULL)
		return EXIT_FAILURE;

	seed = h->seed;
	build_flags = h->build_flags;
	state.entries = h->entries;
	state.alloc_nkeys = h->alloc_nkeys;
	state.build_nkeys = h->build_nkeys;
	state.nentries = (flen - sizeof(*h)) / sizeof(h->entries[0]);

	assert(state.nentries != 0);

	for (size_t i = 0; true; i++) {
		size_t dup[2];

		state.pos = 0;
		res = rgph_build_graph(g, build_flags, seed++,
		    &iterator_func, &state);
		if (res == RGPH_SUCCESS || res != RGPH_AGAIN)
			break;

		state.pos = 0;
		res = rgph_find_duplicates(g, &iterator_func, &state, dup);
		if (res == RGPH_SUCCESS)
			break;

		if (res == RGPH_AGAIN && !dup_checked) {
			if (state.build_nkeys > state.nentries)
				break;
			if (has_duplicates(state.entries, state.nentries))
				break;
			dup_checked = true;
		}

		/* Some hashes are weak. Rotate them periodically. */
		if ((i % 128) == 127) {
			int next_hash = (build_flags & RGPH_HASH_MASK) + 1;

			if (next_hash >= RGPH_HASH_LAST)
				next_hash = 0;

			build_flags &= ~RGPH_HASH_MASK;
			build_flags |= next_hash;
		}
	}

	res = rgph_assign(g, h->assign_flags);
	assert(res != RGPH_SUCCESS || rgph_core_size(g) == 0);

	if (res == RGPH_SUCCESS && (rgph_flags(g) & RGPH_ALGO_BDZ))
		check_bdz(g);

	rgph_free_graph(g);

	return res == RGPH_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
