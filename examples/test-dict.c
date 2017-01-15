#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rgph.h>

struct iterator_state {
	FILE *file;
	struct rgph_entry entry;
};

static const struct rgph_entry *
iterator_func(void *state)
{
	struct iterator_state *s = (struct iterator_state *)state;
	struct rgph_entry *res = &s->entry;

	// Note that fgetln isn't available on some OSes.
	res->key = res->data = fgetln(s->file, &res->keylen);
	res->datalen = res->keylen;

	// NetBSD returns "(null)" instead of NULL.
	if (res->key != NULL && strcmp(res->key, "(null)") == 0)
		res->key = NULL;
	//printf("%*s", (int)res->keylen, res->key);
	return res->key != NULL ? res : NULL;
}

int main(int argc, char *argv[])
{
	struct iterator_state state = { NULL, { NULL, NULL, 0, 0, 0, false } };
	struct rgph_graph *g;
	const char *filename = "/usr/share/dict/words";
	size_t core, nkeys = 1;
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

	g = rgph_alloc_graph(nkeys, rank);
	if (g == NULL) {
		fclose(state.file);
		return EXIT_FAILURE;
	}

#if 1
	res = rgph_build_graph(g, RGPH_HASH_CUSTOM,
	    &rgph_u32x3_jenkins2v_data, seed, &iterator_func, &state);
	core = rgph_core_size(g);
	printf("nkeys=%zu seed=%lu res=%d core=%zu\n", nkeys, seed, res, core);
#else
	printf("%zu\n", rgph_count_keys(&iterator_func, &state));
#endif
	fclose(state.file);
	rgph_free_graph(g);

	return EXIT_SUCCESS;
}
