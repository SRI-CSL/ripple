ARCH ?= $(shell uname -m)

ifeq ($(ARCH), i686)
	ARCH=i386
endif

CFLAGS_x86_64 = 
CFLAGS_i386   = -m32
CFLAGS_armv7l = 

CFLAGS = -std=c11 -Wall -pedantic -Wno-gnu-empty-initializer -O2 -fPIE -D_FORTIFY_SOURCE=2
LDFLAGS = 
INC = -Iinclude/ -Iarch/${ARCH} 
LIBS = -ledit

ifeq ($(ARCH), i386)
	CFLAGS+=-m32
endif


SRC = src/rappel.c src/exedir.c src/common.c src/ptrace.c src/ui.c src/pipe.c src/binary.c src/child.c
SRC_ARCH = arch/${ARCH}/elf.c arch/${ARCH}/display.c arch/${ARCH}/assemble.c arch/${ARCH}/ptrace_reset.c

ALL_SRC = $(SRC) $(SRC_ARCH)

OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

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

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

obj/%.o: arch/${ARCH}/%.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

clean:
	$(RM) obj/*.o *~ $(TARGET)

uninstall:
	$(RM) -rf ~/.rappel
