#pragma once
#ifndef SIMPLE_ARCH_TOOLS_UTILITY_H
#define SIMPLE_ARCH_TOOLS_UTILITY_H

int streql(const char *lhs, const char *rhs);
_Noreturn void failwith(int row, int column, const char *msg, ...);

#endif
