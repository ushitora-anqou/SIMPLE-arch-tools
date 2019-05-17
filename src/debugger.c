#include "debugger.h"
#include "utility.h"

#include <getopt.h>

#define INST_LENGTH 128
static char insts[64 * 1024][INST_LENGTH];
static int ninsts;

void load_insts(FILE *fh)
{
    while (fgets(insts[ninsts++], INST_LENGTH, fh))
        ;
}

_Noreturn void print_usage_and_exit(char *argv0)
{
    fprintf(stderr,
            "Usage: %s [-m initial-membin-path] "
            "[target-assembly-file-path]\n",
            argv0);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char *initial_membin_path = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "m:")) != -1) {
        switch (opt) {
        case 'm':
            initial_membin_path = new_string(optarg);
            break;
        default:
            print_usage_and_exit(argv[0]);
        }
    }

    FILE *fh = NULL;
    if (optind >= argc) print_usage_and_exit(argv[0]);
    fh = fopen(argv[optind], "r");
    if (!fh) {
        fprintf(stderr, "Can't open the specified file: %s\n", argv[optind]);
        exit(EXIT_FAILURE);
    }

    initialize_asm(fh);
    assemble();
    initialize_emu(getIM());

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

    // TODO: assume asm won't use fh after assembling.
    rewind(fh);
    load_insts(fh);

    // TODO: assume IM and DM addresses won't change.
    Word *im = getIM();

    char *search = NULL;
    while (1) {
        int pcIdx = getPC() - im;
        char *inst = insts[pcIdx];

        if (search != NULL && strstr(inst, search) != NULL) {
            // found
            free(search);
            search = NULL;
        }

        if (search == NULL) {
            printf("\n%04X>> %s\n", pcIdx, inst);

            for (int i = 0; i < 8; i++) {
                printf("R%-3d = %04X\t", i, getRegVal(i));
                if (i % 2 == 1) puts("");
            }
            printf("SZCV = %d%d%d%d\n", checkS(), checkZ(), checkC(), checkV());
        }

        while (1) {
            if (search != NULL) break;

            printf("> ");
            fflush(stdout);

            char prompt_buf[256];
            if (fgets(prompt_buf, 256, stdin) == NULL) exit(1);

            if (strlen(prompt_buf) <= 0) goto invalid_command;

            if (prompt_buf[0] == '\n')
                break;
            else if (prompt_buf[0] == 'b') {
                int lineno;
                if (sscanf(prompt_buf, " b %d \n", &lineno) != 1) {
                    puts("ERROR: invalid format");
                    puts("Usage: b <lineno>");
                    continue;
                }
                search = format("# %04d", lineno);
                break;
            }
            else if (prompt_buf[0] == 'q') {
                exit(0);
            }

        invalid_command:
            puts("ERROR: invalid command");
        }

        if (stepEmu()) break;
    }
}
