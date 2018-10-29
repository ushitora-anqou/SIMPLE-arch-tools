CFLAGS=-std=c11 -g3 -O0 -Wall

emulator: emulator.c
	gcc $(CFLAGS) -o $@ $^

assembler: assembler.c
	gcc $(CFLAGS) -o $@ $^

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^

test: test.sh emulator assembler encoder
	./test.sh
