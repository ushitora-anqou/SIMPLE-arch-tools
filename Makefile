CFLAGS=-std=c11 -g3 -O0 -Wall

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^
