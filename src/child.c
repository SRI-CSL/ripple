#include <unistd.h>

#include "common.h"
#include "elf_gen.h"
#include "exedir.h"
#include "ptrace.h"

extern struct options_t options;

const
pid_t gen_child() {
	uint8_t buf[PAGE_SIZE];
	mem_assign(buf, PAGE_SIZE, TRAP, TRAP_SZ);

	uint8_t *elf;
	const size_t elf_sz = gen_elf(&elf, options.start, (uint8_t *)buf, PAGE_SIZE);

	const int exe_fd = write_exe(elf, elf_sz, options.savefile);

	free(elf);

	const pid_t tracee = fork();

	if (tracee < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (tracee == 0) {
		ptrace_child(exe_fd);
		abort();
	}

	// Parent
	close(exe_fd);

	return tracee;
}
