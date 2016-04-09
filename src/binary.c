#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "assemble.h"
#include "common.h"
#include "display.h"
#include "elf_gen.h"
#include "exedir.h"
#include "ptrace.h"
#include "child.h"

#include "binary.h"

#define STDIN_BUF_SZ 64000000 // 64mb
#define BYTECODE_BUF_SZ 64000000 // 64mb

extern struct options_t options;


/* returns true if the child died  */
bool exec_binary(const pid_t pid, const char *binary, const char *offsetstr, const char *bytesstr)
{
  bool fatal = false;
  
  if((bytesstr == NULL) || (offsetstr == NULL)){
    fprintf(stderr, "Need to use -c and -o in conjunction with -b\n");
    return fatal;
  }

  //cleaner to unfold these?
  const off_t offset = parse2uint64(offsetstr);

  //cleaner to unfold these?
  const size_t bytes = parse2uint64(bytesstr);
  if(bytes == 0){ return fatal; }

  uint8_t *const bytecode = xmalloc(bytes);

  const int fd = open(binary, O_RDONLY);

  if(fd == -1){
    perror("Had trouble opening the binary:");
    return fatal;
  }
  
  if(lseek(fd, offset, SEEK_SET) == -1){
    perror("Had trouble setting the offset in the binary:");
    return fatal;
  }
  
  const size_t bytecode_sz = read_data(fd, bytecode, bytes);


  if (bytecode_sz != bytes) {
    fprintf(stderr, "Could not read that many bytes (desired: %zu vs actual: %zu)...\n", bytes, bytecode_sz);
    return fatal;
  }
  
  verbose_printf("Got assembly:\n");
  verbose_dump(bytecode, bytecode_sz, -1);
  
  const pid_t child_pid = (pid == 0 ? gen_child() : pid);

  verbose_printf("child process is %d\n", child_pid);

  struct proc_info_t info = {};
  ARCH_INIT_PROC_INFO(info);
  
  if( ! pid ){
    
    ptrace_launch(child_pid);
    ptrace_cont(child_pid, &info);
    ptrace_reap(child_pid, &info);
    //we are in cmdline mode so print a "before state"
    display(&info);
  }  
  
  ptrace_write(child_pid, (void *)options.start, bytecode, bytecode_sz);
    
  ptrace_reset(child_pid, options.start);
  
  ptrace_cont(child_pid, &info);
  


  if(ptrace_reap(child_pid, &info))
    fatal = true;

  if( !pid && !fatal)
    ptrace_detatch(child_pid, &info);
  
  display(&info);
  
  free(bytecode);

  return fatal;
  
}

void binary_mode()
{
  
  exec_binary(0, options.binary, options.offsetstr, options.bytesstr);

}
