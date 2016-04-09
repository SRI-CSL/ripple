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

struct proc_info_t {
	pid_t pid;

#if defined(__amd64__)
    struct user_regs_struct_amd64 regs_struct;
    struct user_regs_struct_amd64 old_regs_struct;
	struct iovec regs;

    struct user_fpregs_struct_amd64 fpregs_struct;
    struct user_fpregs_struct_amd64 old_fpregs_struct;
	struct iovec fpregs;
#elif defined(__i386__)
    struct user_regs_struct_x86 regs_struct;
    struct user_regs_struct_x86 old_regs_struct;
	struct iovec regs;

    struct user_fpregs_struct_x86 fpregs_struct;
    struct user_fpregs_struct_x86 old_fpregs_struct;
	struct iovec fpregs;

	struct user_fpxregs_struct_x86 fpxregs_struct;
	struct user_fpxregs_struct_x86 old_fpxregs_struct;
	struct iovec fpxregs;
#elif defined(__arm__)
    struct user_regs_arm regs_struct;
    struct user_regs_arm old_regs_struct;
	struct iovec regs;

    struct user_fpregs_arm fpregs_struct;
    struct user_fpregs_arm old_fpregs_struct;
	struct iovec fpregs;
#else
#error "No proc_info_t for architecture"
#endif

	int sig;
	long exit_code;
};

void mem_assign(
		uint8_t *,
		const size_t,
		const uint64_t,
		const size_t);

void* xmalloc(
		size_t);

void* xrealloc(
		void *,
		size_t);

const
size_t read_data(
		const int,
		uint8_t *const,
		const size_t);

void write_data(
		const int,
		const uint8_t *const,
		const size_t);

__attribute__ ((format (printf, 1, 2)))
void verbose_printf(
		const char *const,
		...);

void verbose_dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);

void dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);

uint64_t parse2uint64(
		  const char *const);
