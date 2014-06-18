/**
 * @file    asm.c
 * @author  lhpelosi
 */

#include "asm.h"

#include <stdlib.h>

static void Asm_writeFunction( Function* function, FILE* outputFile );
static void Asm_writeReturn( FILE* outputFile );
static BasicBlock* Block_generateBlocks( Instr* instr );
static Instr* Block_getInstr( BasicBlock* block, int n );
static void Block_computeNextUsage( BasicBlock* block, Function* function );
static void Block_setUsage( Addr addr, int usagePos, int* usageInfo , int localsOffset );



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

   blockList = Block_generateBlocks( function->code );

	for ( BasicBlock* block = blockList ; block ; block = block->next )
   {
      Block_computeNextUsage( block, function );
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



static BasicBlock* Block_generateBlocks( Instr* instr )
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
         block->next = Block_generateBlocks( instr->next );
         break;
      }
      instr = instr->next;
   }

   return block;
}



static Instr* Block_getInstr( BasicBlock* block, int n )
{
   Instr* instr = block->instr;
   while ( n>0 )
   { 
      instr = instr->next;
      n--;
   }
   return instr;
}



static void Block_computeNextUsage( BasicBlock* block, Function* function )
{
   int nLocals = Function_nLocals( function );
   int nTemps = Function_nTemps( function );

   // Estado da tabela de uso na ultima instrucao
   int* usageInfo = (int*) malloc( (nLocals+nTemps) * sizeof(int) );
   int pos = 0;
   while ( pos < nLocals ) // Locais
   {
      usageInfo[ pos ] = block->nInstr; // Live on exit
      pos++;
   }
   while ( pos < nLocals+nTemps ) // Temporarias
   {
      usageInfo[ pos ] = -1; // Not alive e no next use
      pos++;
   }

   // Atualizacao em cada instrucao, do fim para o inicio
   for ( int iInstr = block->nInstr-1 ; iInstr>=0 ; iInstr-- )
   {
      Instr* instr = Block_getInstr( block, iInstr );

      // Guarda o estado atual da tabela junto a instrucao
      instr->usageInfo = usageInfo;
      usageInfo = (int*) malloc( (nLocals+nTemps) * sizeof(int) );
      for ( int i = 0 ; i<nLocals+nTemps ; i++ ) usageInfo[i] = instr->usageInfo[i];

      switch ( instr->op )
      {
         case OP_SET:
         case OP_SET_BYTE:
         case OP_NE:
         case OP_EQ:
         case OP_LT:
         case OP_GT:
         case OP_LE:
         case OP_GE:
         case OP_ADD:
         case OP_SUB:
         case OP_DIV:
         case OP_MUL:
         case OP_NEG:
         case OP_NEW:
         case OP_NEW_BYTE:
         case OP_SET_IDX:
         case OP_SET_IDX_BYTE:
            Block_setUsage( instr->x, -1, usageInfo, nLocals );
            Block_setUsage( instr->y, iInstr, usageInfo, nLocals );
            Block_setUsage( instr->z, iInstr, usageInfo, nLocals );
            break;

         case OP_PARAM:
         case OP_IF:
         case OP_IF_FALSE:
         case OP_RET_VAL:
         case OP_IDX_SET:
         case OP_IDX_SET_BYTE:
            Block_setUsage( instr->x, iInstr, usageInfo, nLocals );
            Block_setUsage( instr->y, iInstr, usageInfo, nLocals );
            Block_setUsage( instr->z, iInstr, usageInfo, nLocals );
            break;

         default:
            break;
      }
   }
   free( usageInfo );
}



static void Block_setUsage( Addr addr, int usagePos, int* usageInfo , int localsOffset )
{
   if ( addr.type == AD_LOCAL )
   {
      usageInfo[ addr.num ] = usagePos;
   }
   else if ( addr.type == AD_TEMP )
   {
      usageInfo[ localsOffset + addr.num ] = usagePos;
   }
}



