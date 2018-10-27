#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t *p;
static uint16_t mem[64 * 1024], res[8];

int eval()
{
    while (1) {
        // TODO: assume that SIMPLE arch is big endian.
        uint8_t *q = (uint8_t *)p;
        uint16_t code = (*q << 8) | *(q + 1);
        int op = code >> 14;

        p++;
        switch (op) {
            case 2: {  // LI
                int op2 = (code >> 11) & 0x07, rb = (code >> 8) & 0x07;
                int8_t d = code & 0xff;
                assert(op2 == 0);
                res[rb] = d;  // r[Rb] = sign_ext(d)
            } break;

            case 3: {
                int rs = (code >> 11) & 0x07, rd = (code >> 8) & 0x07,
                    op3 = (code >> 4) & 0x0f, d = code & 0x0f;

                switch (op3) {
                    case 0x0f:  // HALT
                        return res[0];
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

    return ret;
}
