#include "debugger.h"

static Word *p;
static Word mem[0x5000], reg[8];
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

Word do_sub(Word lhs, Word rhs)
{
    // need 0xffff mask because of integer promotion
    int pl = max(clz(lhs), clz((-rhs) & 0xffff));
    uint32_t uv = ((uint32_t)(lhs - rhs)) & 0xffff;
    int c = pl < clz(uv);

    // check overflow
    int16_t a = lhs, b = rhs;
    int16_t sv = a - b;
    int v = 0;
    if ((a >= 0 && b < 0 && sv < 0) || (a < 0 && b >= 0 && sv >= 0)) v = 1;

    set_cflag(sv, c, v);
    return uv;
}

int stepEmu()
{
    Word code = *p;
    int op = code >> 14;

    p++;
    switch (op) {
    case 0x00: {  // LD
        int ra = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
        int8_t d = code & 0xff;
        reg[ra] = mem[reg[rb] + d];
    } break;

    case 0x01: {  // ST
        int ra = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
        int8_t d = code & 0xff;
        mem[reg[rb] + d] = reg[ra];
    } break;

    case 0x02: {
        int op2 = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
        int8_t d = code & 0xff;
        int16_t d2 = (code & 0x07ff) | ((code & 0x0400) != 0 ? 0xf800 : 0);

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
            p = p + d2;
            break;

        case 0x07: {
            int cond = rb;
            switch (cond) {
            case 0x00:  // BE
                if (Z) p += d;
                break;

            case 0x01:  // BLT
                if (S ^ V) p += d;
                break;

            case 0x02:  // BLE
                if (Z || (S ^ V)) p += d;
                break;

            case 0x03:  // BNE
                if (!Z) p += d;
                break;

            case 0x04:  // BAL
                reg[6] = (Word)(p - mem);
                p += d;
                break;

            case 0x05:  // BR
                p = (Word *)(reg[6] + mem);
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
        int8_t d_signed = (code & 0x0f) | ((code & 0x08) != 0 ? 0xf0 : 0);

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

            reg[rd] = sv;
            set_cflag(reg[rd], c, v);
        } break;

        case 0x01:  // SUB
            reg[rd] = do_sub(reg[rd], reg[rs]);
            break;

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

        case 0x05:  // CMP
            do_sub(reg[rd], reg[rs]);
            break;

        case 0x06:  // MOV
            reg[rd] = reg[rs];
            set_cflag(reg[rd], 0, 0);
            break;

        case 0x07:  // ADDI
            reg[rd] = reg[rd] + d_signed;
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
            int minus = reg[rd] & (1 << 15);
            reg[rd] =
                (reg[rd] >> d) | (minus ? (((1 << d) - 1) << (16 - d)) : 0);
        } break;

        case 0x0d:  // OUT
            printf("output: %d\n", reg[rs]);
            break;

        case 0x0e:  // CMPI
            do_sub(reg[rd], d_signed);
            break;

        case 0x0f:  // HLT
            return 1;

        default:
            assert(0);
        }
    } break;
    }

    return 0;
}

void initialize_emu(Word *im)
{
    p = im;
}

Word getRegVal(int index)
{
    return reg[index];
}

Word *getPC()
{
    return p;
}

Word *getMEM()
{
    return mem;
}

void load_membin(FILE *fh)
{
    int i = 0, ch;
    while ((ch = fgetc(fh)) != EOF) {
        int ch2 = fgetc(fh);
        assert(ch2 != EOF);
        // TODO: assume that SIMPLE arch is big endian.
        mem[i++] = ((Word)ch << 8) | (Word)ch2;
    }
}
