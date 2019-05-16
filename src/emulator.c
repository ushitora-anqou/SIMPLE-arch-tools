#include "debugger.h"
#include "utility.h"

#include <getopt.h>

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
    int quiet_flag = 0, memdump_flag = 0;
    char *initial_membin_path = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "qdm:")) != -1) {
        switch (opt) {
        case 'q':
            quiet_flag = 1;
            break;
        case 'd':
            memdump_flag = 1;
            break;
        case 'm':
            initial_membin_path = new_string(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-q] [-d] [-m initial-membin-path]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

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

    if (!quiet_flag) printf("#insts: %d\n", ninsts);

    return getRegVal(0);
}
