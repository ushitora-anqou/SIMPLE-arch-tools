#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t hexchar2int(char ch)
{
    if ('0' <= ch && ch <= '9')
        ch -= '0';
    else if ('A' <= ch && ch <= 'F')
        ch = 10 + ch - 'A';
    else if ('a' <= ch && ch <= 'f')
        ch = 10 + ch - 'a';
    else
        assert(0);

    return ch;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    char *src = argv[1];
    int size = strlen(src);
    assert(size % 2 == 0);  // assume that strlen(src) is even.

    for (int i = 0; i < size / 2; i++) {
        uint8_t c =
            (hexchar2int(src[i * 2]) << 4) | (hexchar2int(src[i * 2 + 1]));
        putchar(c);
    }
}
