CFLAGS=-std=c11 -g3 -O0 -Wall

all: debugger emulator assembler encoder macro compiler

debugger: debugger.h debugger.c asm.c emu.c utility.c utility.h
	gcc $(CFLAGS) -o $@ debugger.c asm.c emu.c utility.c

emulator: debugger.h emulator.c emu.c utility.c utility.h
	gcc $(CFLAGS) -o $@ emulator.c emu.c utility.c

assembler: debugger.h assembler.c asm.c utility.c utility.h
	gcc $(CFLAGS) -o $@ assembler.c asm.c utility.c

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^

macro: utility.h macro.c utility.c
	gcc $(CFLAGS) -o $@ macro.c utility.c

compiler: utility.h compiler.c utility.c
	gcc $(CFLAGS) -o $@ compiler.c utility.c

test: test.sh emulator assembler encoder macro compiler
	./test.sh

clean:
	rm -rf emulator assembler encoder macro debugger

.PHONY: all test clean
