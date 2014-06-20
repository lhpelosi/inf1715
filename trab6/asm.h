/**
 * @file    asm.h
 * @author  lhpelosi
 */

#ifndef ASM_H
#define ASM_H

#include <stdio.h>
#include "ir.h"

typedef struct BasicBlock_ BasicBlock;
struct BasicBlock_ {
	BasicBlock* next;
	Instr* instr;
   int nInstr;

   int addressDescriptorSize;
   Variable* registerDescriptor[6];
   Variable** addressDescriptor;
};

void Asm_write( IR* program, FILE* outputFile );

#endif

