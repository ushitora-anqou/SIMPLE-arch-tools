#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t Word;
static Word *p;
static Word mem[64 * 1024], res[8];
static Word cflag;
enum { S, Z, C, V };

// TODO: All cflags are set properly?
void set_cflag(int sz, int c, int v)
{
    cflag = 0;
    if (sz < 0) cflag |= (1 << S);
    if (sz == 0) cflag |= (1 << Z);
    if (c) cflag |= (1 << C);
    if (v) cflag |= (1 << V);
}

int clz(uint32_t n)
{
    if (n == 0) return 32;
    return 32 - __builtin_clz((unsigned int)n);
}

uint32_t max(uint32_t lhs, uint32_t rhs)
{
    return lhs > rhs ? lhs : rhs;
}

int eval()
{
    while (1) {
        // TODO: assume that SIMPLE arch is big endian.
        uint8_t *q = (uint8_t *)p;
        Word code = (*q << 8) | *(q + 1);
        int op = code >> 14;

        p++;
        switch (op) {
            case 2: {
                int op2 = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
                int8_t d = code & 0xff;

                switch (op2) {
                    case 0:           // LI
                        res[rb] = d;  // r[Rb] = sign_ext(d)
                        break;

                    default:
                        assert(0);
                }
            } break;

            case 3: {
                int rs = (code >> 11) & 0x07, rd = (code >> 8) & 0x07,
                    op3 = (code >> 4) & 0x0f, d = code & 0x0f;

                switch (op3) {
                    case 0x00: {  // ADD
                        int pl = max(clz(res[rd]), clz(res[rs]));
                        uint32_t uv = res[rd] + res[rs];
                        int c = pl < clz(uv);

                        // check overflow
                        int16_t a = res[rd], b = res[rs];
                        int16_t sv = a + b;
                        int v = 0;
                        if (a >= 0 && b >= 0 && sv < 0 ||
                            a < 0 && b < 0 && sv >= 0)
                            v = 1;

                        res[rd] = uv;
                        set_cflag(res[rd], c, v);
                    } break;

                    case 0x06:  // MOV
                        res[rd] = res[rs];
                        set_cflag(res[rd], 0, 0);
                        break;

                    case 0x0f:  // HALT
                        return res[0];

                    default:
                        assert(0);
                }
            } break;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 2);

    char *filename = argv[1];

    // read the file and assign to p.
    FILE *fh = fopen(filename, "r");
    assert(fh != NULL);
    fseek(fh, 0, SEEK_END);
    long filesize = ftell(fh);
    fseek(fh, 0, SEEK_SET);
    fread(mem, filesize, 1, fh);
    fclose(fh);

    p = mem;
    int ret = eval();

    // if (cflag & (1 << S)) fprintf(stderr, "S ");
    // if (cflag & (1 << Z)) fprintf(stderr, "Z ");
    // if (cflag & (1 << C)) fprintf(stderr, "C ");
    // if (cflag & (1 << V)) fprintf(stderr, "V ");
    // fprintf(stderr, "\n");

    return ret;
}
