#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <histedit.h>

#include "assemble.h"
#include "common.h"
#include "display.h"
#include "exedir.h"
#include "elf_gen.h"
#include "ptrace.h"
#include "child.h"
#include "binary.h"
#include "teval.h"

#include "ui.h"

extern struct options_t options;
extern int exiting;

static const char* delimiters = " \t\n";

int in_block;

static
char const* prompt(
		   EditLine *const e)
{
  if (in_block)
    return "_> ";
  else
    return "> ";
}

static
void help()
{
  printf("Commands:\n");
  printf(".quit                               - quit\n");
  printf(".help                               - display this help\n");
  printf(".info                               - display registers\n");
  printf(".begin                              - start a block, input will not be assembled/run until '.end'\n");
  printf(".end                                - assemble and run the prior block\n");
  printf(".showmap                            - shortcut for cat /proc/<pid>/maps\n");
  printf(".read <address> [amount]            - read <amount> bytes of data from address using ptrace [16]\n");
  printf(".write <address> <data>             - write data starting at address using ptrace\n");
  printf(".execute <binary> <offset> <length> - execute the sequence of machine code of specified length at offset in binary\n");
  printf(".teval <file>                       - execute the test scenario in file (appending result to file)\n");
}

static
void ui_read(
	     const pid_t child_pid,
	     const char *line)
{
  char *dupline = strdup(line);

  if (!dupline) {
    perror("strdup");
    return;
  }

  char *saveptr;

  const char *dotread = strtok_r(dupline, " ", &saveptr);

  if (!dotread || strcasecmp(dotread, ".read"))
    goto bail;

  const char *addr_str = strtok_r(NULL, " ", &saveptr);

  if (!addr_str)
    goto bail;

  errno = 0;
  const unsigned long addr = strtoul(addr_str, NULL, 0);

  if (addr == ULONG_MAX && errno) {
    perror("strtoul");
    goto bail;
  }

  const char *sz_str = strtok_r(NULL, " ", &saveptr);

  unsigned long sz = 0x10;

  if (sz_str && strlen(sz_str)) {
    errno = 0;
    sz = strtoul(sz_str, NULL, 0);

    if (sz == ULONG_MAX && errno) {
      perror("strtoul");
      goto bail;
    }
  }

  uint8_t *buf = xmalloc(sz);

  if (!ptrace_read(child_pid, (void *)addr, buf, sz))
    dump(buf, sz, addr);

  free(buf);

 bail:
  free(dupline);
}

static
bool ui_teval(
	      const pid_t child_pid,
	      const char *line)
{
  bool fatal = false;
  char *dupline = strdup(line);

  if (!dupline) {
    perror("strdup");
    return fatal;
  }

  char *saveptr;

  const char *dotteval = strtok_r(dupline, delimiters, &saveptr);

  if (!dotteval || strcasecmp(dotteval, ".teval")){
    printf("dotteval = %s\n", dotteval);
    goto bail;
  }

  const char *filein_str = strtok_r(NULL, delimiters, &saveptr);

  if (!filein_str){
    fprintf(stderr, ".teval requires an input file!\n");
    goto bail;
  }
	
  const char *fileout_str = strtok_r(NULL, delimiters, &saveptr);

  if (!fileout_str){
    fprintf(stderr, ".teval requires an output file!\n");
    goto bail;
  }
	
  verbose_printf("teval gonna work on '%s' and write to '%s'\n", 
		 filein_str, fileout_str);


  fatal = teval(child_pid, filein_str, fileout_str);

 bail:
  free(dupline);
  return fatal;
}


static
bool ui_execute(
		const pid_t child_pid,
		const char *line)
{
  char *dupline = strdup(line);
  bool fatal = false;
  
  if (!dupline) {
    perror("strdup");
    return fatal;
  }
  
  char *saveptr;
  
  const char *cmdread = strtok_r(dupline, " \t\n", &saveptr);
  
  if (!cmdread || strcasecmp(cmdread, ".execute"))
    goto bail;
  
  const char *binary = strtok_r(NULL, " \t\n", &saveptr);
  
  if(binary == NULL)
    goto bail;
  
  const char* offsetstr =  strtok_r(NULL, " \t\n", &saveptr);
  
  if(offsetstr == NULL)
    goto bail;

  const char* bytesstr =  strtok_r(NULL, " \t\n", &saveptr);
  
  if(bytesstr == NULL)
    goto bail;
  
  
  fatal = exec_binary(child_pid, binary, offsetstr, bytesstr);
  
  
 bail:
  free(dupline);

  return fatal;
  
}

void interact(
	      const char *const argv_0)
{
  EditLine *const el = el_init(argv_0, stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  History *const hist = history_init();
  if (!hist) {
    fprintf(stderr, "Could not initialize history\n");
    exit(EXIT_FAILURE);
  }

  HistEvent ev;
  history(hist, &ev, H_SETSIZE, 100);

  el_set(el, EL_HIST, history, hist);

  const pid_t child_pid = gen_child();

  verbose_printf("child process is %d\n", child_pid);

  if (options.verbose) help();

  char buf[PAGE_SIZE];
  size_t buf_sz = 0;
  int end = 0;
  bool child_died = false;

  struct proc_info_t info = {};
  ARCH_INIT_PROC_INFO(info);

  ptrace_launch(child_pid);
  ptrace_cont(child_pid, &info);
  ptrace_reap(child_pid, &info);

  display(&info);

  for (;;) {
    int count;
    const char *const line = el_gets(el, &count);

    if (count == -1) {
      perror("el_gets");
      exit(EXIT_FAILURE);
    }

    // count is 0 == ^d
    if (!count || strcasestr(line, ".quit") || strcasestr(line, ".exit")) break;

    // We have input, add it to the our history
    history(hist, &ev, H_ENTER, line);

    // If we start with a ., we have a command
    if (line[0] == '.') {
      if (strcasestr(line, "help")) {
	help();
	continue;
      }

      if (strcasestr(line, "info")) {
	display(&info);
	continue;
      }

      if (strcasestr(line, "showmap")) {
	char cmd[PATH_MAX] = { 0 };
	snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps", child_pid);

	if (system(cmd))
	  fprintf(stderr, "sh: %s failed\n", cmd);

	continue;
      }


      if (strcasestr(line, "read")) {
	ui_read(child_pid, line);
	continue;
      }


      if (strcasestr(line, "teval")) {
	child_died = ui_teval(child_pid, line);
	if(child_died)
	  break;
	else 
	  continue;
      }

      if (strcasestr(line, "execute")) {
	child_died = ui_execute(child_pid, line);
	if(child_died)
	  break;
	else 
	  continue;
      }

      if (strcasestr(line, "write")) {
	continue;
      }

      if (strcasestr(line, "begin")) {
	in_block = 1;
	continue;
      }

      // Note the lack of continue. Need to fall through...
      if (strcasestr(line, "end")) {
	in_block = 0;
	end = 1;
      }
    }

    if (buf_sz + count > sizeof(buf)) {
      printf("Buffer full (max: 0x%lx), please use '.end'\n", sizeof(buf));
      continue;
    }

    // Since we fell through, we want to avoid adding adding .end to our buffer
    if (!end) {
      memcpy(buf + buf_sz, line, count);
      buf_sz += count;
    }

    if (!in_block) {
      verbose_printf("Trying to assemble(%zu):\n%s", buf_sz, buf);

      uint8_t bytecode[PAGE_SIZE];
      const size_t bytecode_sz = assemble(bytecode, sizeof(bytecode), buf, buf_sz);

      memset(buf, 0, sizeof(buf));
      buf_sz = 0;
      end    = 0;

      verbose_printf("Got asm(%zu):\n", bytecode_sz);
      verbose_dump(bytecode, bytecode_sz, -1);

      if (!bytecode_sz) {
	fprintf(stderr, "'%s' assembled to 0 length bytecode\n", buf);
	continue;
      }

      ptrace_write(child_pid, (void *)options.start, bytecode, bytecode_sz);
      ptrace_reset(child_pid, options.start, NULL);

      ptrace_cont(child_pid, &info);

      if (ptrace_reap(child_pid, &info)) {
	child_died = true;
	break;
      }

      display(&info);
    }
  }

  if (!child_died)
    ptrace_detatch(child_pid, &info);

  printf("\n");

  history_end(hist);
  el_end(el);
}
