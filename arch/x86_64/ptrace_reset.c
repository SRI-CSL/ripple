#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <elf.h> // NT_PRSTATUS

#include "common.h"

void ptrace_reset(
		const pid_t child_pid,
		const unsigned long start,
		const struct proc_info_t *const info
)
{
	struct user_regs_struct_amd64 regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.rip = start;

	if(info != NULL){
	  /* this gives 
     
	     ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0: Input/output error

	     unsigned long long int rsp = regs_struct.rsp ;
	     regs_struct = info->regs_struct;
	     regs_struct.rip = start;
	     regs_struct.rsp = rsp;
	     
	     so what is wrong with the rest of info->regs_struct?
	     the man page says about PTRACE_SETREGS:
	     
	     "As for PTRACE_POKEUSER, some general-purpose register
	     modifications may be disallowed."
	     
	     so we use a white list. will have to grok this better ...
	     
	  */

	  regs_struct.rip = start;
	  
	  regs_struct.rax = info->regs_struct.rax;
	  regs_struct.rbx = info->regs_struct.rbx;
	  regs_struct.rcx = info->regs_struct.rcx;
	  regs_struct.rdx = info->regs_struct.rdx;
	  
	  regs_struct.rsi = info->regs_struct.rsi;
	  regs_struct.rdi = info->regs_struct.rdi;
	  
	  regs_struct.r8 = info->regs_struct.r8;
	  regs_struct.r9 = info->regs_struct.r9;
	  regs_struct.r10 = info->regs_struct.r10;
	  regs_struct.r11 = info->regs_struct.r11;
	  regs_struct.r12 = info->regs_struct.r12;
	  regs_struct.r13 = info->regs_struct.r13;
	  regs_struct.r14 = info->regs_struct.r14;
	  regs_struct.r15 = info->regs_struct.r15;

	  regs_struct.rbp = info->regs_struct.rbp;

	  regs_struct.eflags = info->regs_struct.eflags;
	}

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
}





