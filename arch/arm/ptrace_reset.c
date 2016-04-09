#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <elf.h> // NT_PRSTATUS

#include "common.h"


void ptrace_reset(
		const pid_t child_pid,
		const unsigned long start)
{
	struct user_regs_arm regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.uregs[15] = start;

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
}
