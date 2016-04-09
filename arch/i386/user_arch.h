
#define TRAP 0xcc // int3
#define TRAP_SZ 1

#define REGFMT "0x%08"  PRIx32


#define ARCH_INIT_PROC_INFO(i)			\
	do {\
		i.regs    = (struct iovec) { .iov_base = &i.regs_struct,    .iov_len = sizeof(i.regs_struct) }; \
		i.fpregs  = (struct iovec) { .iov_base = &i.fpregs_struct,  .iov_len = sizeof(i.fpregs_struct) }; \
		i.fpxregs = (struct iovec) { .iov_base = &i.fpxregs_struct, .iov_len = sizeof(i.fpxregs_struct) }; \
	} while (0)

