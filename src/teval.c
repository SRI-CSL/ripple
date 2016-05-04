#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <inttypes.h>

#include <string.h>

#include <sys/types.h>
#include <sys/ptrace.h>

#include <sys/uio.h>

#include "user.h"
#include "common.h"


extern struct options_t options;

static struct proc_info_t info = {};

#define MAX_INSTRUCTION_LENGTH   64
// we will read that values into &info->regs_struct
// and the instruction into tinstr[64]
static char tinstr[MAX_INSTRUCTION_LENGTH];

static bool parse_teval_file(const char *tfile);

static bool parse_teval_line(const char *line);

static bool parse_teval_cmd(const char *lhs, const char *rhs);

/* returns true if the child died  */
bool teval(const pid_t pid,  const char *tfile)
{
  bool fatal = false;
  
  ARCH_INIT_PROC_INFO(info);

  bool ok = parse_teval_file(tfile);

  if(ok){





  }

  return fatal;
}


static bool parse_teval_file(const char *tfile){
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
    if( !  parse_teval_line(line) ){
      success = false;
      break;
    }
  }

  fclose(fp);

  free(line);
    
  return success;
}


static bool parse_teval_line(const char *line){
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

  success = parse_teval_cmd(lhs, rhs);

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



static bool parse_teval_cmd(const char *lhs, const char *rhs){

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

      fprintf(stderr, "Saw reg[%d] called %s with string %s and value %" PRIu64 " (lhs = %s)\n", index, regname, rhs, val, lhs);
      

      return true;
    }
  }


  return false;
}
