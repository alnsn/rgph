#include "t_util.h"

int exit_status = EXIT_SUCCESS;

int main()
{

	rgph_test_jenkins2();
	rgph_test_murmur32();
	rgph_test_murmur32s();
	rgph_test_xxh32s();
	rgph_test_xxh64s();
	rgph_test_t1ha64s();
	rgph_test_fastdiv();
	return exit_status;
}
