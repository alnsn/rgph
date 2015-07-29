#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rgph.h>

struct iterator_state {
	FILE *file;
	struct rgph_entry entry;
};

struct rgph_entry *iterator_func(void *state) {
	struct iterator_state *s = (struct iterator_state *)state;
	struct rgph_entry *res = &s->entry;

	res->key = fgetln(s->file, &res->keylen);
	//printf("%*s", (int)res->keylen, res->key);
	return res;
}

int main(int argc, char *argv[]) {
	struct iterator_state state = { 0 };
	struct rgph_graph *g;
	const char *filename = "/usr/share/dict/words";
	size_t nkeys = 1;
	unsigned long seed = 0;
	int res;
	const int rank = RGPH_RANK3;

	if (argc > 1)
		nkeys = strtoul(argv[1], NULL, 0);
	if (argc > 2)
		seed = strtoul(argv[2], NULL, 0);
	if (argc > 3)
		filename = argv[3];

	state.file = fopen(filename, "r");
	if (state.file == NULL)
		return EXIT_FAILURE;

	g = rgph_alloc_graph(nkeys, RGPH_HASH_JENKINS2 | rank);
	if (g == NULL) {
		fclose(state.file);
		return EXIT_FAILURE;
	}

	res = rgph_build_graph(g, &iterator_func, &state, seed);
	printf("nkeys=%zu seed=%lu res=%d\n", nkeys, seed, res);
	fclose(state.file);
	rgph_free_graph(g);

	return EXIT_SUCCESS;
}
