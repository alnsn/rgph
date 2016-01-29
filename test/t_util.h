#ifndef RGPH_TEST_UTIL_H_INCLUDED
#define RGPH_TEST_UTIL_H_INCLUDED

#include <err.h>
#include <stdint.h>
#include <stdlib.h>

extern int exit_status;

#define REQUIRE(x) if (!(x)) { \
	errx(EXIT_FAILURE, "%s:%u (in %s): %s\nAborted",  \
	    __FILE__, __LINE__, __func__, #x); }

#define CHECK(x) if (!(x)) { \
	exit_status = EXIT_FAILURE; \
	warnx("%s:%u (in %s): %s", __FILE__, __LINE__, __func__, #x); }


/*
 * Tests.
 */
void rgph_test_jenkins2(void);
void rgph_test_murmur32(void);
void rgph_test_murmur32s(void);
void rgph_test_xxh32s(void);
void rgph_test_xxh64s(void);

#endif /* #ifndef RGPH_TEST_UTIL_H_INCLUDED */
