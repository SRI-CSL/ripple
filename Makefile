ARCH ?= $(shell uname -m)

ifeq ($(ARCH), i686)
	ARCH=i386
endif

CFLAGS_x86_64 = -DREGFMT=REGFMT64 -DARCH_INIT_PROC_INFO=AMD64_INIT_PROC_INFO
CFLAGS_i386   = -DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=X86_INIT_PROC_INFO \
		-m32
CFLAGS_armv7l = -DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=ARM_INIT_PROC_INFO

CFLAGS = -std=c11 -Wall -pedantic -Wno-gnu-empty-initializer $(CFLAGS_$(ARCH)) -O2 -fPIE -D_FORTIFY_SOURCE=2
LDFLAGS = 
INC = -Iinclude/ 
LIBS = -ledit

SRC = rappel.c exedir.c common.c ptrace.c ui.c pipe.c binary.c child.c
SRC_ARCH = arch/${ARCH}/elf.c arch/${ARCH}/display.c arch/${ARCH}/assemble.c arch/${ARCH}/ptrace_reset.c

ALL_SRC = $(SRC) $(SRC_ARCH)

OBJ = $(patsubst %.c, obj/%.o, $(SRC))

OBJ_ARCH = $(patsubst arch/${ARCH}/%.c, obj/%.o, $(SRC_ARCH))

ALL_OBJ = $(OBJ) $(OBJ_ARCH)

TARGET = bin/rappel

.PHONY: clean

all: $(TARGET)
	@echo Done.

debug: CFLAGS += -g
debug: $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(ALL_OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $(ALL_OBJ) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

obj/%.o: arch/${ARCH}/%.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

clean:
	$(RM) obj/*.o *~ $(TARGET)

uninstall:
	$(RM) -rf ~/.rappel
