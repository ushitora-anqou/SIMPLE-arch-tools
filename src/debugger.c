#include "debugger.h"
#include "utility.h"

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
