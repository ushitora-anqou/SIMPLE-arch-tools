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
    assert(scanf(" R%d , %d", lhs, rhs) == 2);
    assert_reg(*lhs);
    assert_byte(*rhs);
}

void read_reg_reg(int *lhs, int *rhs)
{
    assert(scanf(" R%d , R%d", lhs, rhs) == 2);
    assert_reg(*lhs);
    assert_reg(*rhs);
}

void read_imm(int *v)
{
    assert(scanf(" %d", v) == 1);
    assert_byte(*v);
}

void read_reg_mem(int *ra, int *rb, int *d)
{
    assert(scanf(" R%d , %d ( R%d )", ra, d, rb) == 3);
    assert_reg(*ra);
    assert_reg(*rb);
    assert_byte(*d);
}

int main()
{
    char op[256];
    while (scanf("%s", op) != EOF) {
        if (streql(op, "ADD")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 0, 0);
        }

        if (streql(op, "SUB")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 1, 0);
        }

        if (streql(op, "AND")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 2, 0);
        }

        if (streql(op, "OR")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 3, 0);
        }

        if (streql(op, "XOR")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 4, 0);
        }

        if (streql(op, "CMP")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 5, 0);
        }

        if (streql(op, "MOV")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 6, 0);
        }

        if (streql(op, "SLL")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 8, d);
        }

        if (streql(op, "SLR")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 9, d);
        }

        if (streql(op, "SRL")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 10, d);
        }

        if (streql(op, "SRA")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 11, d);
        }

        if (streql(op, "LD")) {
            int ra, rb, d;
            read_reg_mem(&ra, &rb, &d);
            put2338(0, ra, rb, d);
        }

        if (streql(op, "ST")) {
            int ra, rb, d;
            read_reg_mem(&ra, &rb, &d);
            put2338(1, ra, rb, d);
        }

        if (streql(op, "LI")) {
            int rb, d;
            read_reg_imm(&rb, &d);
            put2338(2, 0, rb, d);
        }

        if (streql(op, "B")) {
            int d;
            read_imm(&d);
            put2338(2, 4, 0, d);
        }

        if (streql(op, "BE")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 0, d);
        }

        if (streql(op, "BNE")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 3, d);
        }

        if (streql(op, "HLT")) put23344(3, 0, 0, 15, 0);
    }
}
