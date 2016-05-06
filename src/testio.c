#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <inttypes.h>

#include <string.h>

#include "common.h"


#define MAX_INSTRUCTION_LENGTH   64

static 
char tinstr[MAX_INSTRUCTION_LENGTH];

static 
bool parse_teval_file(const char *tfile, struct proc_info_t *const info);

static 
bool parse_teval_line(const char *line, struct proc_info_t *const info);

static 
bool parse_teval_cmd(const char *lhs, const char *rhs, struct proc_info_t *const info);



const
bool file2info(
	       const char *const filein,
	       char **const instr,
	       struct proc_info_t *info){


  /* might not be the first call */
  memset(&tinstr, 0, MAX_INSTRUCTION_LENGTH);

  assert(filein != NULL);
  assert(instr != NULL);
  assert(info != NULL);

  const bool ok = parse_teval_file(filein, info);
  
  if(ok){

    const size_t tinstr_sz = strlen(tinstr);

    if(tinstr_sz == 0){ 
      fprintf(stderr, "No instruction found in %s\n", filein);
      return false;
    }

    *instr = strdup(tinstr);
    return true;
  }

  return false;
}

static bool parse_teval_file(const char *tfile, struct proc_info_t *const info){
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  bool success = true;
  FILE* fp = fopen(tfile, "r");

  if(fp == NULL){
    perror("fopen");
    return false;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if( !  parse_teval_line(line, info) ){
      success = false;
      break;
    }
  }

  fclose(fp);

  free(line);
    
  return success;
}
static bool parse_teval_line(const char *line, struct proc_info_t *const info){
  bool success = false;
  char *dupline = strdup(line);


  if (!dupline) {
    perror("strdup");
    return success;
  }

  char *saveptr;

  const char *lhs = strtok_r(dupline, "=\n", &saveptr);

  if(!lhs) 
    goto bail;
   
  const char *rhs = strtok_r(NULL, "=\n", &saveptr);

  if (!rhs)
    goto bail;

  success = parse_teval_cmd(lhs, rhs, info);

 bail:
  free(dupline);
  return success;
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


static inline void set_reg_slot(struct user_regs_struct_amd64* regs, int slot,  const uint64_t val){
  const uintptr_t slotptr = (uintptr_t)(regs) + sizeof(unsigned long long int) * slot;
  *(unsigned long long int *)slotptr = val;
}

static inline unsigned long long int get_reg_slot(const struct user_regs_struct_amd64* regs, int slot){
  const uintptr_t slotptr = (uintptr_t)(regs) + sizeof(unsigned long long int) * slot;
  return *(unsigned long long int *)slotptr;
}

const
bool info2file(
	       const char *const fileout,
	       const char *const header,
	       const struct proc_info_t *const info)
{
  
  assert(fileout != NULL);
  assert(header != NULL);
  assert(info != NULL);

  FILE* fp = fopen(fileout, "a+");

  if(fp == NULL){
    perror("fopen");
    return false;
  }

  fprintf(fp, "# %s:\n", header);

  int index = 0;
  char* regname;

  while((regname = register_slots[index]) != NULL){
    unsigned long long int old_val = get_reg_slot(&info->old_regs_struct, index);
    unsigned long long int new_val = get_reg_slot(&info->regs_struct, index);
    if(old_val != new_val){
      fprintf(fp, "%s="REGFMT"\n", regname, new_val);
    }
    index++;
  }
  
  fclose(fp);
  return true;
}

static bool parse_teval_cmd(const char *lhs, const char *rhs, struct proc_info_t *const info){

  //handle the instruction case
  if( !strcasecmp(lhs, "instr") ){

    const size_t len = strlen(rhs);
    
    if( len == 0 ){
      fprintf(stderr, "Empty instruction :-(\n");
      return false;
    }

    if ( strlen(tinstr) != 0){
      fprintf(stderr, "Only one instruction per file please\n");
      return false;
    }

    if ( len >= MAX_INSTRUCTION_LENGTH ){
      fprintf(stderr, "Instruction %s too long %zu > %d\n", rhs, len, MAX_INSTRUCTION_LENGTH);
      return false;
    }
    
    strncpy(tinstr, rhs, len);
    
    tinstr[len] = '\0';
    
    return true;

  } else {
    //handle the regsiters etc 
    int index = 0;
    char* regname;
    while((regname = register_slots[index]) != NULL){
      
      if( strcasecmp(lhs, regname) ){
	index++;
	continue;
      }
      
      const uint64_t val = parse2uint64(rhs);

      set_reg_slot(&info->regs_struct, index,  val);
      
      return true;
    }
  }


  return false;
}
