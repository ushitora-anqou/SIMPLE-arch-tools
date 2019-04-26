CFLAGS=-std=c11 -g3 -O0 -Wall

all: emulator assembler encoder macro

emulator: emulator.c
	gcc $(CFLAGS) -o $@ $^

assembler: assembler.c
	gcc $(CFLAGS) -o $@ $^

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^

macro: macro.c
	gcc $(CFLAGS) -o $@ $^

test: test.sh emulator assembler encoder macro
	./test.sh

clean:
	rm -rf emulator assembler encoder macro

.PHONY: all test clean
