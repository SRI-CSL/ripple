const
int ptrace_write(
		const pid_t,
		const void *const,
		const uint8_t *const,
		size_t);

const
int ptrace_read(
		const pid_t,
		const void *const,
		void *const,
		const size_t);


void 
ptrace_peek(const pid_t child_pid);

//now defined in arch/${ARCH}/ptrace_reset.c
void ptrace_reset(
		const pid_t child_pid,
		const unsigned long start,
		const struct proc_info_t *const info
		);

void ptrace_child(
		const int);

void ptrace_launch(
		const pid_t);

const
int ptrace_reap(
		const pid_t,
		struct proc_info_t *const);

void ptrace_cont(const pid_t,
		struct proc_info_t *const);

void ptrace_detatch(
		const pid_t child_pid,
		struct proc_info_t *const);
