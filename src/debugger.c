#include "debugger.h"

#define INST_LENGTH 128
static char insts[64 * 1024][INST_LENGTH];
static int ninsts;

void load_insts(FILE *fh)
{
    while (fgets(insts[ninsts++], INST_LENGTH, fh))
        ;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *fh = fopen(argv[1], "r");
    assert(fh);

    initialize_asm(fh);
    assemble();
    initialize_emu(getIM());

    // TODO: assume asm won't use fh after assembling.
    rewind(fh);
    load_insts(fh);

    // TODO: assume IM and DM addresses won't change.
    Word *im = getIM();

    while (1) {
        int pcIdx = getPC() - im;
        char *inst = insts[pcIdx];
        printf("%04X>> %s\n", pcIdx, inst);

        for (int i = 0; i < 8; i++) {
            printf("R%-3d = %04X\t", i, getRegVal(i));
            if (i % 2 == 1) puts("");
        }
        printf("SZCV = %d%d%d%d\n", checkS(), checkZ(), checkC(), checkV());

        // wait for enter
        getchar();

        if (stepEmu()) break;
    }
}
