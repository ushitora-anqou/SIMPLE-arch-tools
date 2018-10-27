#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char *p;

int eval() { return 0; }

int main(int argc, char **argv)
{
    assert(argc == 2);

    char *filename = argv[1];

    // read the file and assign to p.
    FILE *fh = fopen(filename, "r");
    assert(fh != NULL);
    fseek(fh, 0, SEEK_END);
    long filesize = ftell(fh);
    p = (char *)malloc(filesize);
    fseek(fh, 0, SEEK_SET);
    fread(p, filesize, 1, fh);
    fclose(fh);

    int ret = eval();

    free(p);

    return ret;
}
