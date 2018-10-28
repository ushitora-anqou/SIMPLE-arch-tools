#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

void putword(uint16_t n)
{
    // TODO: assume that SIMPLE arch is big endian.
    putchar(n >> 8);
    putchar(n);
}

void put23344(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
{
    a &= 0x03;
    b &= 0x07;
    c &= 0x07;
    d &= 0x0f;
    e &= 0x0f;
    putword((a << 14) | (b << 11) | (c << 8) | (d << 4) | e);
}

void put2338(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    put23344(a, b, c, d >> 4, d);
}

int main()
{
    char op[256];
    while (scanf("%s", op) != EOF) {
        if (streql(op, "HLT")) put23344(3, 0, 0, 15, 0);
    }
}
