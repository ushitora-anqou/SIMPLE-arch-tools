#include "debugger.h"

static int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

int main(int argc, char **argv)
{
    int mifmode = argc >= 2 && streql(argv[1], "-mif");

    initialize_asm(stdin);
    assemble();
    Word *im = getIM();
    int size = getIMSize();

    for (int i = 0; i < size; i++) {
        if (mifmode) {
            printf("%04X : %04X;\n", i, im[i]);
        }
        else {
            putchar(im[i] >> 8);
            putchar(im[i]);
        }
    }
}
