#include <stdint.h>

#if defined(__amd64__)
#define BITSTR "[bits 64]\n"
#elif defined(__i386__)
#define BITSTR "[bits 32]\n"
#endif

const
size_t assemble(
		uint8_t *const,
		const size_t,
		const char *const,
		const size_t);
