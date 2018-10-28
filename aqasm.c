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

void assert_reg(int reg)
{
    assert(0 <= reg && reg < 8);
}

void assert_byte(int n)
{
    assert(-128 <= n && n <= 255);
}

void read_reg_imm(int *lhs, int *rhs)
{
    scanf(" R%d , %d", lhs, rhs);
}

int main()
{
    char op[256];
    while (scanf("%s", op) != EOF) {
        /*
        if (streql(op, "MOV")) {
            int rd, rs;
            scanf("%d,%d", &rd, &rs);
            assert(0 <= rd && rd < 8);
            assert(0 <= rs && rs < 8);
            put23344(3, rs, rd, 6, 0);
        }
        */

        if (streql(op, "LI")) {
            int rb, d;
            read_reg_imm(&rb, &d);
            assert_reg(rb);
            assert_byte(d);
            put2338(2, 0, rb, d);
        }

        if (streql(op, "HLT")) put23344(3, 0, 0, 15, 0);
    }
}
