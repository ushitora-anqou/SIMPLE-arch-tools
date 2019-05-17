#pragma once
#ifndef SIMPLE_ARCH_TOOLS_DEBUGGER_H
#define SIMPLE_ARCH_TOOLS_DEBUGGER_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t Word;

// emu.c
void initialize_emu(Word *im);
Word getRegVal(int index);
Word *getPC();
Word *getMEM();
int checkS();
int checkZ();
int checkC();
int checkV();
// return 1 when HLT, return 0 otherwise.
int stepEmu();
void load_membin(FILE *fh);

// asm.c
void initialize_asm(FILE *srcfh);
Word *getIM();
int getIMSize();
void assemble();

#endif
