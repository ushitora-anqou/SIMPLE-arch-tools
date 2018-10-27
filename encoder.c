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
                putword((3 << 14) | (rs << 11) | (rd << 8) | (op3 << 4) | d);
                break;
            }

            case 'c': {
                // 10 op2(3) Rb(3) d(8)
                int op2, rb, d;
                scanf("%d %d %d ]", &op2, &rb, &d);
                putword((2 << 14) | (op2 << 11) | (rb << 8) | d);
                break;
            }

            default:
                assert(0);
        }
    }
}
