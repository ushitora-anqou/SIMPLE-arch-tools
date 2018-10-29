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
    while (1) {
        char op;
        if (scanf(" %c [", &op) == EOF) break;
        // fprintf(stderr, "'%c'\n", op);
        switch (op) {
        case 'a': {
            // 11 Rs(3) Rd(3) op3(4) d(4)
            int rs, rd, op3, d;
            scanf("%d %d %d %d ]", &rs, &rd, &op3, &d);
            put23344(3, rs, rd, op3, d);
            break;
        }

        case 'b': {
            // op1(2) Ra(3) Rb(3) d(8)
            int op1, ra, rb, d;
            scanf("%d %d %d %d ]", &op1, &ra, &rb, &d);
            put2338(op1, ra, rb, d);
            break;
        }

        case 'c': {
            // 10 op2(3) Rb(3) d(8)
            int op2, rb, d;
            scanf("%d %d %d ]", &op2, &rb, &d);
            put2338(2, op2, rb, d);
            break;
        }

        case 'd': {
            // 10 111 cond(3) d(8)
            int cond, d;
            scanf("%d %d ]", &cond, &d);
            put2338(2, 7, cond, d);
            break;
        }

        default:
            fprintf(stderr, "'%c'\n", op);
            assert(0);
        }
    }
}
