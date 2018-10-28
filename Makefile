CFLAGS=-std=c11 -g3 -O0 -Wall

aqemu: aqemu.c
	gcc $(CFLAGS) -o $@ $^

aqasm: aqasm.c
	gcc $(CFLAGS) -o $@ $^

encoder: encoder.c
	gcc $(CFLAGS) -o $@ $^

test: test.sh aqemu aqasm encoder
	./test.sh
