#include "debugger.h"

static Word im[64 * 1024];

int main(int argc, char **argv)
{
    int ch, i = 0;
    while ((ch = getchar()) != EOF) {
        int ch2 = getchar();
        assert(ch2 != EOF);
        // TODO: assume that SIMPLE arch is big endian.
        im[i++] = ((Word)ch << 8) | (Word)ch2;
    }

    initialize_emu(im);
    while (!stepEmu())
        ;

    return getRegVal(0);
}
