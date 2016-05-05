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
#include "assemble.h"
#include "display.h"
#include "ptrace.h"
#include "testio.h"


extern struct options_t options;


/* returns true if the child died  */
bool teval(const pid_t child_pid,  const char *tfilein,  const char *tfileout)
{
  bool  fatal = false;
  char* tinstr = NULL;

  struct proc_info_t info = {};

  memset(&info, 0, sizeof(info));

  ARCH_INIT_PROC_INFO(info);

  info.pid       = child_pid;
  info.sig       = -1;
  info.exit_code = -1;

  
  bool ok = file2info(tfilein, &tinstr, &info);
    
  if(ok){

    const size_t tinstr_sz = strlen(tinstr);

    verbose_printf("Trying to assemble(%zu):\n%s\n", tinstr_sz, tinstr);

    uint8_t bytecode[PAGE_SIZE];
    const size_t bytecode_sz = assemble(bytecode, sizeof(bytecode), tinstr, tinstr_sz);

    verbose_printf("Got asm(%zu):\n", bytecode_sz);
    verbose_dump(bytecode, bytecode_sz, -1);

    if (!bytecode_sz) {
      fprintf(stderr, "'%s' assembled to 0 length bytecode\n", tinstr);
      goto bail;
    }
    
    ptrace_write(child_pid, (void *)options.start, bytecode, bytecode_sz);

    ptrace_reset(child_pid, options.start, &info);

    /* just for debugging */
    ptrace_peek(child_pid);

    ptrace_cont(child_pid, &info);

    if (ptrace_reap(child_pid, &info)) {
      fatal = true;
    }
    
    display(&info);

    if( ! info2file(tfileout, &info) ){
      
    }

    
  }

 bail:
  
  free(tinstr);
  
  return fatal;
}




