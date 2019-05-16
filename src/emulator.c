#include "debugger.h"
#include "utility.h"

static Word im[64 * 1024];

void load_membin(const char *filepath, Word *dm)
{
    FILE *fh = fopen(filepath, "r");
    assert(fh != NULL);

    int i = 0, ch;
    while ((ch = fgetc(fh)) != EOF) {
        int ch2 = fgetc(fh);
        assert(ch2 != EOF);
        // TODO: assume that SIMPLE arch is big endian.
        dm[i++] = ((Word)ch << 8) | (Word)ch2;
    }
}

int main(int argc, char **argv)
{
    int memdump_flag = argc == 2;
    char *initial_membin_path = NULL;
    if (argc == 2) initial_membin_path = argv[1];

    int ch, i = 0;
    while ((ch = getchar()) != EOF) {
        int ch2 = getchar();
        assert(ch2 != EOF);
        // TODO: assume that SIMPLE arch is big endian.
        im[i++] = ((Word)ch << 8) | (Word)ch2;
    }

    initialize_emu(im);

    if (initial_membin_path) {
        // load membin to place in the initial data memory
        load_membin(initial_membin_path, getMEM());
    }

    int ninsts = 0;
    while (!stepEmu()) ninsts++;

    if (memdump_flag) {
        Word *mem = getMEM();
        for (int i = 0; i < 0x1C00; i++) printf("%04X : %04X\n", i, mem[i]);
    }

    printf("#insts: %d\n", ninsts);

    return getRegVal(0);
}
