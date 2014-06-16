/**
 * @file    asm.c
 * @author  lhpelosi
 */

#include "asm.h"

#include <stdlib.h>

static void Asm_writeFunction( Function* function, FILE* outputFile );
static void Asm_writeReturn( FILE* outputFile );
static BasicBlock* generateBlocks( Instr* instr );



void Asm_write( IR* program, FILE* outputFile )
{
	fprintf( outputFile, ".data\n" );
	for ( String* s = program->strings ; s ; s = s->next )
   {
		fprintf( outputFile, "%s:\t.string %s\n", s->name, s->value );
	}
	for ( Variable* v = program->globals ; v ; v = v->next )
   {
		fprintf( outputFile, "%s:\t.int 0\n", v->name );
	}
	fprintf( outputFile, "\n.text\n.globl main\n" );
	for ( Function* fun = program->functions ; fun ; fun = fun->next )
   {
		Asm_writeFunction( fun, outputFile );
	}
}



static void Asm_writeFunction( Function* function, FILE* outputFile )
{
   int nVariables = 0;
   BasicBlock* blockList = NULL;

   // Conta as variaveis
	for ( Variable* v = function->locals ; v ; v = v->next ) nVariables++;
	for ( Variable* v = function->temps ; v ; v = v->next ) nVariables++;
   nVariables -= function->nArgs;

	fprintf( outputFile, "\n%s:", function->name );
   // Registro de ativacao
   fprintf( outputFile, "\tpushl\t%%ebp\n"
                        "\tmovl\t%%esp, %%ebp\n" );
   // Aloca espaco das variaveis
   fprintf( outputFile, "\tsubl\t$%d, %%esp\n", 4*nVariables );
   // Salva os registradores
   fprintf( outputFile, "\tpushl\t%%ebx\n"
                        "\tpushl\t%%esi\n"
                        "\tpushl\t%%edi\n" );

   blockList = generateBlocks( function->code );

	for ( BasicBlock* block = blockList ; block ; block = block->next )
   {
      // Asm_writeBlock
	}
   Asm_writeReturn( outputFile );
}



static void Asm_writeReturn( FILE* outputFile )
{
   // Recupera os registradores
   fprintf( outputFile, "\tpopl\t%%edi\n"
                        "\tpopl\t%%esi\n"
                        "\tpopl\t%%ebx\n" );
   // Retorno de registro de ativacao
   fprintf( outputFile, "\tmovl\t%%ebp, %%esp\n"
                        "\tpopl\t%%ebp\n"
                        "\tret\n" );
}



static BasicBlock* generateBlocks( Instr* instr )
{
   BasicBlock* block;
   if ( !instr ) return NULL;

   block = (BasicBlock*) malloc( sizeof(BasicBlock) );
   block->next = NULL;
   block->instr = instr;
   block->nInstr = 0;

   while ( instr )
   {
      (block->nInstr)++;
      if ( instr->next == NULL ||
           instr->next->op == OP_LABEL ||
           instr->op == OP_GOTO ||
           instr->op == OP_IF ||
           instr->op == OP_IF_FALSE )
      {
         block->next = generateBlocks( instr->next );
         break;
      }
      instr = instr->next;
   }

   return block;
}

