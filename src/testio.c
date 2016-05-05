#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "common.h"

const
bool file2info(
	       const char *const filein,
	       char **const instr,
	       struct proc_info_t *info){


  return true;
}

/* these are in the order they appear in the struct */
static char* register_slots[] = 
  {
    "r15",
    "r14",
    "r13",
    "r12",
    "rbp",
    "rbx",
    "r11",
    "r10",
    "r9",
    "r8",
    "rax",
    "rcx",
    "rdx",
    "rsi",
    "rdi",
    "orig_rax",
    "rip",
    "cs",
    "rflags",  //called eflags in the struct { ... }
    "rsp",
    "ss",
    "fs_base",
    "gs_base",
    "ds",
    "es",
    "fs",
    "gs",
    NULL
  };

static inline unsigned long long int get_reg_slot(const struct user_regs_struct_amd64* regs, int slot){
  const uintptr_t slotptr = (uintptr_t)(regs) + sizeof(unsigned long long int) * slot;
  return *(unsigned long long int *)slotptr;
}

const
bool info2file(
	       const char *const fileout,
	       const struct proc_info_t *const info)
{
  
  assert(fileout != NULL);
  assert(info != NULL);

  FILE* fp = fopen(fileout, "w");

  if(fp == NULL){
    perror("fopen");
    return false;
  }

  int index = 0;
  char* regname;

  while((regname = register_slots[index]) != NULL){
    unsigned long long int val = get_reg_slot(&info->regs_struct, index);
    fprintf(fp, "%s="REGFMT"\n", regname, val);
    index++;
  }
  
  fclose(fp);
  return true;
}
