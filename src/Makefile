CFLAGS=-std=c11 -g3 -O0 -Wall

all: debugger emulator assembler encoder macro

debugger: debugger.h debugger.c asm.c emu.c
	gcc $(CFLAGS) -o $@ debugger.c asm.c emu.c

emulator: debugger.h emulator.c emu.c
	gcc $(CFLAGS) -o $@ emulator.c emu.c

assembler: debugger.h assembler.c asm.c
	gcc $(CFLAGS) -o $@ assembler.c asm.c

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^

macro: macro.c
	gcc $(CFLAGS) -o $@ $^

test: test.sh emulator assembler encoder macro
	./test.sh

clean:
	rm -rf emulator assembler encoder macro debugger

.PHONY: all test clean
