#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <elf.h> // NT_PRSTATUS

#include "common.h"

void ptrace_reset(
		const pid_t child_pid,
		const unsigned long start)
{
	struct user_regs_struct_amd64 regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.rip = start;

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
}


void ptrace_set(
		const pid_t child_pid,
		const unsigned long start,
		const struct proc_info_t *const info
		)
{
  struct user_regs_struct_amd64 regs_struct = {}; //info->regs_struct;
  struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };
  
  REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
  
  regs_struct.rip = start;
  regs_struct.rax = info->regs_struct.rax;
  /*
    ok so this works so what is wrong with the rest of info->regs_struct?
    the man page says about PTRACE_SETREGS:

    "As for PTRACE_POKEUSER, some general-purpose register
    modifications may be disallowed."
  */
  REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

}



