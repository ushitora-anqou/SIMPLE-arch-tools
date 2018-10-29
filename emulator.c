#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t Word;
static Word *p;
static Word mem[64 * 1024], reg[8];
static Word cflag;
enum { FLAG_S, FLAG_Z, FLAG_C, FLAG_V };

// TODO: All cflags are set properly?
void set_cflag(int sz, int c, int v)
{
    cflag = 0;
    if (sz < 0) cflag |= (1 << FLAG_S);
    if (sz == 0) cflag |= (1 << FLAG_Z);
    if (c) cflag |= (1 << FLAG_C);
    if (v) cflag |= (1 << FLAG_V);
}

int checkS()
{
    return (cflag & (1 << FLAG_S)) != 0;
}

int checkZ()
{
    return (cflag & (1 << FLAG_Z)) != 0;
}

int checkC()
{
    return (cflag & (1 << FLAG_C)) != 0;
}

int checkV()
{
    return (cflag & (1 << FLAG_V)) != 0;
}

#define S checkS()
#define Z checkZ()
#define C checkC()
#define V checkV()

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
        case 0x00: {  // LD
            int ra = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
            int8_t d = code & 0xff;
            uint8_t *addr = (uint8_t *)(&mem[reg[rb] + d]);
            reg[ra] = (*addr << 8) | (*(addr + 1));
        } break;

        case 0x01: {  // ST
            int ra = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
            int8_t d = code & 0xff;
            uint8_t *addr = (uint8_t *)(&mem[reg[rb] + d]);
            *addr = reg[ra] >> 8;
            *(addr + 1) = reg[ra] & 0xff;
        } break;

        case 0x02: {
            int op2 = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
            int8_t d = code & 0xff;

            switch (op2) {
            case 0x00:        // LI
                reg[rb] = d;  // r[Rb] = sign_ext(d)
                break;

            case 0x01:  // reserved
            case 0x02:  // reserved
            case 0x03:  // reserved
            case 0x05:  // reserved
            case 0x06:  // reserved
                break;

            case 0x04:  // B
                // PC = PC + 1 + sign_ext(d)
                // note that '+ 1' is executed by 'p++' above.
                p = p + d;
                break;

            case 0x07: {
                int cond = rb;
                switch (cond) {
                case 0x00:  // BE
                    if (Z) p += d;
                    break;

                    /*
                case 0x01:  // BLT
                    if (S ^ V) p += d;
                    break;

                case 0x02:  // BLE
                    if (Z || (S ^ V)) p += d;
                    break;
                    */

                case 0x03:  // BNE
                    if (!Z) p += d;
                    break;
                }
            } break;

            default:
                assert(0);
            }
        } break;

        case 0x03: {
            int rs = (code >> 11) & 0x07, rd = (code >> 8) & 0x07,
                op3 = (code >> 4) & 0x0f, d = code & 0x0f;

            switch (op3) {
            case 0x00: {  // ADD
                int pl = max(clz(reg[rd]), clz(reg[rs]));
                uint32_t uv = reg[rd] + reg[rs];
                int c = pl < clz(uv);

                // check overflow
                int16_t a = reg[rd], b = reg[rs];
                int16_t sv = a + b;
                int v = 0;
                if ((a >= 0 && b >= 0 && sv < 0) || (a < 0 && b < 0 && sv >= 0))
                    v = 1;

                reg[rd] = uv;
                set_cflag(reg[rd], c, v);
            } break;

            case 0x01: {  // SUB
                // need 0xffff mask because of integer promotion
                int pl = max(clz(reg[rd]), clz((-reg[rs]) & 0xffff));
                uint32_t uv = ((uint32_t)(reg[rd] - reg[rs])) & 0xffff;
                int c = pl < clz(uv);

                // check overflow
                int16_t a = reg[rd], b = reg[rs];
                int16_t sv = a - b;
                int v = 0;
                if ((a >= 0 && b < 0 && sv < 0) || (a < 0 && b >= 0 && sv >= 0))
                    v = 1;

                set_cflag(uv, c, v);
                reg[rd] = uv;
            } break;

            case 0x02:  // AND
                reg[rd] = reg[rd] & reg[rs];
                set_cflag(reg[rd], 0, 0);
                break;

            case 0x03:  // OR
                reg[rd] = reg[rd] | reg[rs];
                set_cflag(reg[rd], 0, 0);
                break;

            case 0x04:  // XOR
                reg[rd] = reg[rd] ^ reg[rs];
                set_cflag(reg[rd], 0, 0);
                break;

            case 0x05: {  // CMP
                // need 0xffff mask because of integer promotion
                int pl = max(clz(reg[rd]), clz((-reg[rs]) & 0xffff));
                uint32_t uv = ((uint32_t)(reg[rd] - reg[rs])) & 0xffff;
                int c = pl < clz(uv);

                // check overflow
                int16_t a = reg[rd], b = reg[rs];
                int16_t sv = a - b;
                int v = 0;
                if ((a >= 0 && b < 0 && sv < 0) || (a < 0 && b >= 0 && sv >= 0))
                    v = 1;

                set_cflag(uv, c, v);
            } break;

            case 0x06:  // MOV
                reg[rd] = reg[rs];
                set_cflag(reg[rd], 0, 0);
                break;

            case 0x07:  // reserved
                break;

            case 0x08:  // SLL
                reg[rd] = reg[rd] << d;
                break;

            case 0x09:  // SLR
                reg[rd] = (reg[rd] << d) | (reg[rd] >> (16 - d));
                break;

            case 0x0a:  // SRL
                reg[rd] = reg[rd] >> d;
                break;

            case 0x0b: {  // SRA
                int plus = reg[rd] & (1 << 15);
                reg[rd] =
                    (reg[rd] >> d) | (plus ? 0 : (((1 << d) - 1) << (16 - d)));
            } break;

            case 0x0f:  // HALT
                return reg[0];

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