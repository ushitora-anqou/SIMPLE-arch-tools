#pragma once
#ifndef SIMPLE_ARCH_TOOLS_UTILITY_H
#define SIMPLE_ARCH_TOOLS_UTILITY_H

#include <stdarg.h>

int streql(const char *lhs, const char *rhs);
_Noreturn void error_at(int row, int column, const char *msg);
char *format(const char *src, ...);
char *vformat(const char *src, va_list ap);
char *new_string(const char *src);

#endif
