#include "debugger.h"
#include "utility.h"

#include <getopt.h>

static Word im[20000];

int main(int argc, char **argv)
{
    int quiet_flag = 0, memdump_flag = 0, force_flag = 0,
        register_dump_flag = 0;
    char *initial_membin_path = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "qdfrm:")) != -1) {
        switch (opt) {
        case 'q':
            quiet_flag = 1;
            break;
        case 'd':
            memdump_flag = 1;
            break;
        case 'f':
            force_flag = 1;
            break;
        case 'm':
            initial_membin_path = new_string(optarg);
            break;
        case 'r':
            register_dump_flag = 1;
            break;
        default:
            fprintf(stderr,
                    "Usage: %s [-q] [-d] [-f] [-r] [-m initial-membin-path]\n",
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
    // set 0xBEEF pattern to the rest of IM to find SEGV at runtime
    for (; i < sizeof(im) / sizeof(Word); i++) im[i] = 0xBEEF;

    initialize_emu(im);

    if (initial_membin_path) {
        // load membin to place in the initial data memory
        FILE *fh = fopen(initial_membin_path, "r");
        if (!fh) {
            fprintf(stderr,
                    "Can't open the specified initial membin file: %s\n",
                    initial_membin_path);
            exit(EXIT_FAILURE);
        }
        load_membin(fh);
    }

    int ninsts = 0;
    while (!stepEmu()) {
        ninsts++;
        if (!force_flag && *getPC() == 0xBEEF) {
            fprintf(
                stderr,
                "Potentially invalid instruction (0xBEEF) was detected. "
                "Maybe you forgot to use HLT in your program. If you really "
                "want to force to continue, set command-line option "
                "'-f'.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (memdump_flag) {
        Word *mem = getMEM();
        for (int i = 0; i < 0x5000; i++) printf("%04X : %04X\n", i, mem[i]);
    }

    if (!quiet_flag) printf("#insts: %d\n", ninsts);

    if (register_dump_flag) {
        for (int i = 0; i < 8; i++) {
            printf("R%-3d = %04X\t", i, getRegVal(i));
            if (i % 2 == 1) puts("");
        }
        printf("SZCV = %d%d%d%d\n", checkS(), checkZ(), checkC(), checkV());
    }

    return getRegVal(0);
}
