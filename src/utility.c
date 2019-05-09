#include "utility.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

_Noreturn void failwith(int row, int column, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    if (row != -1) fprintf(stderr, "\e[1mstdin:%d:%d: ", row, column);
    fprintf(stderr, "\e[31mERROR:\e[m ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
