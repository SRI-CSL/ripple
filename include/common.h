#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include <sys/uio.h>

#include "user.h"
#include "user_arch.h"

#define REQUIRE(x) \
	do {\
		if (!(x)) { \
			perror(#x); \
			exit(EXIT_FAILURE); \
		}\
	} while (0)


struct options_t {
  unsigned long start;
  int raw;
  int verbose;
  int allregs;
  int passsig;
  const char *savefile;
  const char *binary;
  const char *offsetstr;
  const char *bytesstr;
};

#define REGFMT64 "0x%016" PRIx64
#define REGFMT32 "0x%08"  PRIx32
#define REGFMT16 "0x%04"  PRIx16
#define REGFMT8  "0x%02"  PRIx8

extern 
void mem_assign(
		uint8_t *,
		const size_t,
		const uint64_t,
		const size_t);

extern 
void* xmalloc(
		size_t);

extern 
void* xrealloc(
		void *,
		size_t);

extern const
size_t read_data(
		const int,
		uint8_t *const,
		const size_t);

extern void write_data(
		const int,
		const uint8_t *const,
		const size_t);

__attribute__ ((format (printf, 1, 2)))
extern 
void verbose_printf(
		const char *const,
		...);

extern 
void verbose_dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);

extern 
void dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);

extern 
uint64_t parse2uint64(
		  const char *const);
