#include "debugger.h"

static FILE *fh;
static Word im[64 * 1024], nim;

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

void putword(Word n)
{
    im[nim++] = n;
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
    assert(fscanf(fh, " R%d , %d", lhs, rhs) == 2);
    assert_reg(*lhs);
    assert_byte(*rhs);
}

void read_reg_reg(int *lhs, int *rhs)
{
    assert(fscanf(fh, " R%d , R%d", lhs, rhs) == 2);
    assert_reg(*lhs);
    assert_reg(*rhs);
}

void read_imm(int *v)
{
    assert(fscanf(fh, " %d", v) == 1);
    assert_byte(*v);
}

void read_reg_mem(int *ra, int *rb, int *d)
{
    assert(fscanf(fh, " R%d , %d ( R%d )", ra, d, rb) == 3);
    assert_reg(*ra);
    assert_reg(*rb);
    assert_byte(*d);
}

void assemble()
{
    char op[256];
    while (fscanf(fh, "%s", op) != EOF) {
        if (streql(op, "ADD")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 0, 0);
        }
        else if (streql(op, "SUB")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 1, 0);
        }
        else if (streql(op, "AND")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 2, 0);
        }
        else if (streql(op, "OR")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 3, 0);
        }
        else if (streql(op, "XOR")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 4, 0);
        }
        else if (streql(op, "CMP")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 5, 0);
        }
        else if (streql(op, "MOV")) {
            int rd, rs;
            read_reg_reg(&rd, &rs);
            put23344(3, rs, rd, 6, 0);
        }
        else if (streql(op, "SLL")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 8, d);
        }
        else if (streql(op, "SLR")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 9, d);
        }
        else if (streql(op, "SRL")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 10, d);
        }
        else if (streql(op, "SRA")) {
            int rd, d;
            read_reg_imm(&rd, &d);
            put23344(3, 0, rd, 11, d);
        }
        else if (streql(op, "LD")) {
            int ra, rb, d;
            read_reg_mem(&ra, &rb, &d);
            put2338(0, ra, rb, d);
        }
        else if (streql(op, "ST")) {
            int ra, rb, d;
            read_reg_mem(&ra, &rb, &d);
            put2338(1, ra, rb, d);
        }
        else if (streql(op, "LI")) {
            int rb, d;
            read_reg_imm(&rb, &d);
            put2338(2, 0, rb, d);
        }
        else if (streql(op, "B")) {
            int d;
            read_imm(&d);
            put2338(2, 4, 0, d);
        }
        else if (streql(op, "BE")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 0, d);
        }
        else if (streql(op, "BLT")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 1, d);
        }
        else if (streql(op, "BLE")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 2, d);
        }
        else if (streql(op, "BNE")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 3, d);
        }
        else if (streql(op, "BAL")) {
            int d;
            read_imm(&d);
            put2338(2, 7, 4, d);
        }
        else if (streql(op, "BR")) {
            put2338(2, 7, 5, 0);
        }
        else if (streql(op, "HLT")) {
            put23344(3, 0, 0, 15, 0);
        }
        else {
            assert(0);
        }
    }
}

void initialize_asm(FILE *srcfh)
{
    fh = srcfh;
}

Word *getIM()
{
    return im;
}
