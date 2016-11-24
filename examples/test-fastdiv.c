/*
 * Check that rgph_fastdiv_prepare() generates the same parameters
 * as libdivide_u32_gen() and libdivide_u32_branchfree_gen().
 *
 * It takes a while for this test to finish because it iterates
 * from 2 to UINT32_MAX twice.
 *
 * gcc -O2 -I ../lib -I ${PATH_TO_LIBDIVIDE:?} test-fastdiv.c ../lib/fastdiv.c
 *
 */
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include <libdivide.h>

#include <rgph_fastdiv.h>

int main()
{
	uint32_t i, m, magic;
	uint8_t s1, s2, more;
	int branchfree;

	for (branchfree = 0; branchfree <= 1; branchfree++) {
		for (i = UINT32_MAX; i > 1; i--) {
			rgph_fastdiv_prepare(i, &m, &s1, &s2, branchfree);

			if (branchfree) {
				struct libdivide_u32_branchfree_t dv;

				dv = libdivide_u32_branchfree_gen(i);
				magic = dv.magic;
				more = dv.more;
			} else {
				struct libdivide_u32_t dv;

				dv = libdivide_u32_gen(i);
				magic = dv.magic;
				more = dv.more;
			}

			if ((i & (i - 1)) == 0) {
				assert(m < 2);
				assert(s1 == branchfree);
				assert((1u << (s1 + s2)) == i);
			} else if (branchfree) {
				assert(m == magic);
				assert(s1 == 1);
				assert(s2 == more);
			} else if (more & LIBDIVIDE_ADD_MARKER) {
				assert(m == magic);
				assert(s1 == 1);
				assert(s2 == (more & LIBDIVIDE_32_SHIFT_MASK));
			} else {
				assert(m == magic);
				assert(s1 == 0);
				assert(s2 == more);
			}
		}
	}
}
