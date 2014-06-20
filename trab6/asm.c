/**
 * @file    asm.c
 * @author  lhpelosi
 */

#include "asm.h"

#include <stdlib.h>

#define ASM_ADDR_BUFFER_SIZE 128

static void Asm_writeFunction( Function* function, FILE* outputFile );
static void Asm_writeBlock( BasicBlock* block, Function* function, FILE* outputFile );
static void Asm_writeInstr( Instr* instr, Function* function, FILE* outputFile );
static void Asm_writeBinOpArit( char* op, Instr* instr, Function* function, FILE* outputFile );
static void Asm_writeBinOpComp( char* op, Instr* instr, Function* function, FILE* outputFile );
static void Asm_writeNew( int size, Instr* instr, Function* function, FILE* outputFile );
static void Asm_writeReturn( FILE* outputFile );
static void Asm_getAddr( Addr addr, Function* function, char* output );
static void Asm_translateAddr( Addr addr, Function* function, char* output );
static int Asm_generateLabel();
static BasicBlock* Block_generateBlocks( Instr* instr, Function* function );
static Instr* Block_getInstr( BasicBlock* block, int n );
static void Block_computeNextUsage( BasicBlock* block, Function* function );
static void Block_setUsage( Addr addr, int usagePos, int* usageInfo , int localsOffset );



void Asm_write( IR* program, FILE* outputFile )
{
   // Strings e globais
	fprintf( outputFile, ".data\n" );
	for ( String* s = program->strings ; s ; s = s->next )
   {
		fprintf( outputFile, "%s:\t.string %s\n", s->name, s->value );
	}
	for ( Variable* v = program->globals ; v ; v = v->next )
   {
		fprintf( outputFile, ".comm\t%s, 4\n", v->name );
	}
	fprintf( outputFile, "\n.text\n" );
   // Imprime as funcoes
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
   nVariables += Function_nLocals( function );
   nVariables += Function_nTemps( function );
   nVariables -= function->nArgs;

	fprintf( outputFile, "\n.globl %s\n"
                        ".type\t%s, @function\n"
                        "%s:\n",
                        function->name,
                        function->name,
                        function->name );
   // Registro de ativacao
   fprintf( outputFile, "\tpushl\t%%ebp\n"
                        "\tmovl\t%%esp, %%ebp\n" );
   // Aloca espaco das variaveis
   fprintf( outputFile, "\tsubl\t$%d, %%esp\n", 4*nVariables );
   // Salva os registradores
   fprintf( outputFile, "\tpushl\t%%ebx\n"
                        "\tpushl\t%%esi\n"
                        "\tpushl\t%%edi\n" );

   blockList = Block_generateBlocks( function->code, function );

	for ( BasicBlock* block = blockList ; block ; block = block->next )
   {
      Block_computeNextUsage( block, function );
      Asm_writeBlock( block, function, outputFile );
	}

   // Caso nao tenha um ret no final da funcao
   for ( Instr* instr = function->code ; instr ; instr = instr->next )
      if ( instr->next == NULL && instr->op != OP_RET && instr->op != OP_RET_VAL  )
         Asm_writeReturn( outputFile );
}



static void Asm_writeBlock( BasicBlock* block, Function* function, FILE* outputFile )
{
   int iInstr = 0;
   for ( Instr* instr = block->instr ; iInstr < block->nInstr ; instr = instr->next, iInstr++ )
      Asm_writeInstr( instr, function, outputFile );
}



static void Asm_writeInstr( Instr* instr, Function* function, FILE* outputFile )
{
   // Buffers para guardar as strings representando os enderecos
   char bufferX[ASM_ADDR_BUFFER_SIZE];
   char bufferY[ASM_ADDR_BUFFER_SIZE];
   char bufferZ[ASM_ADDR_BUFFER_SIZE];

   switch ( instr->op )
   {
      case OP_LABEL :
         fprintf( outputFile, "%s:\n", instr->x.str );
         break;

      case OP_GOTO :
         fprintf( outputFile, "\tjmp\t%s\n", instr->x.str );
         break;

      case OP_PARAM :
         Asm_getAddr( instr->x, function, bufferX );
         fprintf( outputFile, "\tpushl\t%s\n", bufferX );
         break;

      case OP_CALL :
         fprintf( outputFile, "\tcall\t%s\n"
                              "\taddl\t$%d, %%esp\n", // Desaloca os parametros
                              instr->x.str,
                              4 * instr->y.num );
         break;

      case OP_RET :
         Asm_writeReturn( outputFile );
         break;

      case OP_RET_VAL :
         Asm_getAddr( instr->x, function, bufferX );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n", bufferX );
         Asm_writeReturn( outputFile );
         break;

      case OP_IF :
         Asm_getAddr( instr->x, function, bufferX );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tcmpl\t$0, %%eax\n"
                              "\tjne\t%s\n",
                              bufferX,
                              instr->y.str );
         break;

      case OP_IF_FALSE :
         Asm_getAddr( instr->x, function, bufferX );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tcmpl\t$0, %%eax\n"
                              "\tje\t%s\n",
                              bufferX,
                              instr->y.str );
         break;

      case OP_NE : Asm_writeBinOpComp( "jne", instr, function, outputFile ); break;
      case OP_EQ : Asm_writeBinOpComp( "je", instr, function, outputFile ); break;
      case OP_LT : Asm_writeBinOpComp( "jl", instr, function, outputFile ); break;
      case OP_GT : Asm_writeBinOpComp( "jg", instr, function, outputFile ); break;
      case OP_LE : Asm_writeBinOpComp( "jle", instr, function, outputFile ); break;
      case OP_GE : Asm_writeBinOpComp( "jge", instr, function, outputFile ); break;

      case OP_ADD : Asm_writeBinOpArit( "addl", instr, function, outputFile ); break;
      case OP_SUB : Asm_writeBinOpArit( "subl", instr, function, outputFile ); break;
      case OP_MUL : Asm_writeBinOpArit( "imul", instr, function, outputFile ); break;

      case OP_DIV :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         Asm_getAddr( instr->z, function, bufferZ );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\tcltd\n"
                              "\tidiv\t%%ecx\n"
                              "\tmovl\t%%eax, %s\n",
                              bufferY,
                              bufferZ,
                              bufferX );
         break;
         
      case OP_NEG :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tnegl\t%%eax\n"
                              "\tmovl\t%%eax, %s\n",
                              bufferY,
                              bufferX );
         break;

      case OP_NEW : Asm_writeNew( 4, instr, function, outputFile ); break;
      case OP_NEW_BYTE : Asm_writeNew( 1, instr, function, outputFile ); break;
         
      case OP_SET :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%%eax, %s\n",
                              bufferY,
                              bufferX );
         break;
         
      case OP_SET_BYTE :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovsbl\t%%eax, %s\n",
                              bufferY,
                              bufferX );
         break;
         
      case OP_SET_IDX :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         Asm_getAddr( instr->z, function, bufferZ );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\timul\t$4, %%eax\n"
                              "\taddl\t%%ecx, %%eax\n"
                              "\tmovl\t(%%eax), %%eax\n"
                              "\tmovl\t%%eax, %s\n",
                              bufferZ,
                              bufferY,
                              bufferX );
         break;
         
      case OP_SET_IDX_BYTE :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         Asm_getAddr( instr->z, function, bufferZ );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\taddl\t%%ecx, %%eax\n"
                              "\tmovl\t(%%eax), %%eax\n"
                              "\tmovsbl\t%%eax, %s\n",
                              bufferZ,
                              bufferY,
                              bufferX );
         break;
         
      case OP_IDX_SET :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         Asm_getAddr( instr->z, function, bufferZ );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\timul\t$4, %%eax\n"
                              "\taddl\t%%ecx, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\tmovl\t%%ecx, (%%eax)\n",
                              bufferY,
                              bufferX,
                              bufferZ );
         break;
         
      case OP_IDX_SET_BYTE :
         Asm_getAddr( instr->x, function, bufferX );
         Asm_getAddr( instr->y, function, bufferY );
         Asm_getAddr( instr->z, function, bufferZ );
         fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\taddl\t%%ecx, %%eax\n"
                              "\tmovl\t%s, %%ecx\n"
                              "\tmovb\t%%ecx, (%%eax)\n",
                              bufferY,
                              bufferX,
                              bufferZ );
         break;
         
      default:
         break;
   }
}



static void Asm_writeBinOpArit( char* op, Instr* instr, Function* function, FILE* outputFile )
{
   // Buffers para guardar as strings representando os enderecos
   char bufferX[ASM_ADDR_BUFFER_SIZE];
   char bufferY[ASM_ADDR_BUFFER_SIZE];
   char bufferZ[ASM_ADDR_BUFFER_SIZE];
   Asm_getAddr( instr->x, function, bufferX );
   Asm_getAddr( instr->y, function, bufferY );
   Asm_getAddr( instr->z, function, bufferZ );

   fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                        "\tmovl\t%s, %%ecx\n"
                        "\t%s\t%%ecx, %%eax\n"
                        "\tmovl\t%%eax, %s\n",
                        bufferY,
                        bufferZ,
                        op,
                        bufferX );
}



static void Asm_writeBinOpComp( char* op, Instr* instr, Function* function, FILE* outputFile )
{
   // Buffers para guardar as strings representando os enderecos
   char bufferX[ASM_ADDR_BUFFER_SIZE];
   char bufferY[ASM_ADDR_BUFFER_SIZE];
   char bufferZ[ASM_ADDR_BUFFER_SIZE];
   Asm_getAddr( instr->x, function, bufferX );
   Asm_getAddr( instr->y, function, bufferY );
   Asm_getAddr( instr->z, function, bufferZ );
   int label = Asm_generateLabel();

   fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                        "\tmovl\t%s, %%ecx\n"
                        "\tcmpl\t%%ecx, %%eax\n"
                        "\t%s\t.LComp_%d_a\n"
                        "\tmovl\t$0, %%eax\n"
                        "\tjmp\t.LComp_%d_b\n"
                        ".LComp_%d_a:\n"
                        "\tmovl\t$1, %%eax\n"
                        ".LComp_%d_b:\n"
                        "\tmovl\t%%eax, %s\n",
                        bufferY,
                        bufferZ,
                        op, label,
                        label,
                        label,
                        label,
                        bufferX );
}




static void Asm_writeNew( int size, Instr* instr, Function* function, FILE* outputFile )
{
   // Buffers para guardar as strings representando os enderecos
   char bufferX[ASM_ADDR_BUFFER_SIZE];
   char bufferY[ASM_ADDR_BUFFER_SIZE];
   Asm_getAddr( instr->x, function, bufferX );
   Asm_getAddr( instr->y, function, bufferY );
   fprintf( outputFile, "\tmovl\t%s, %%eax\n"
                        "\timul\t$%d, %%eax\n"
                        "\tpushl\t%%eax\n"
                        "\tcall\tmalloc\n"
                        "\taddl\t$4, %%esp\n"
                        "\tmovl\t%%eax, %s\n",
                        bufferY,
                        size,
                        bufferX );
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



static void Asm_getAddr( Addr addr, Function* function, char* output )
{
   // TODO: Tentar um registrador

   // Se tiver que ser o proprio endereco em memoria
   Asm_translateAddr( addr, function, output );
}



static void Asm_translateAddr( Addr addr, Function* function, char* output )
{
   int nLocals = Function_nLocals( function );
   int nArgs = function->nArgs;
   int pos = 0;

   switch ( addr.type )
   {
      // Pega variaveis globais e strings de .data
      case AD_GLOBAL :
      case AD_STRING :
         sprintf( output, "$%s", addr.str );
         break;

      // Para as variaveis locais verifica se sao parametros ou internas
      case AD_LOCAL :
         if ( addr.num < nArgs ) // Eh parametro da funcao
         {
            pos = addr.num + 2; // +2 pelo deslocamento do %ebp e do end de retorno
         }
         else // Eh variavel interna
         {
            pos = addr.num - nArgs + 1; // +1 pelo deslocamento do %ebp
            pos = -pos; // Pois os enderecos crescem pra baixo
         }
         pos *= 4; // Variaveis de 4 bytes
         sprintf( output, "%d(%%ebp)", pos );
         break;

      // As variaveis temporarias sao alocadas apos as locais
      case AD_TEMP :
         pos = nLocals - nArgs + addr.num + 1; // +1 pelo deslocamento do %ebp
         pos = -pos; // Pois os enderecos crescem pra baixo
         pos *= 4; // Variaveis de 4 bytes
         sprintf( output, "%d(%%ebp)", pos );
         break;

      // Constantes numericas
      case AD_NUMBER :
         sprintf( output, "$%d", addr.num );
         break;

      default :
         sprintf( output, "inv" );
         break;
   }
}



static int Asm_generateLabel()
{
   static int nLabel = 0;
   nLabel++;
   return nLabel;
}



static BasicBlock* Block_generateBlocks( Instr* instr, Function* function )
{
   BasicBlock* block;
   if ( !instr ) return NULL;

   block = (BasicBlock*) malloc( sizeof(BasicBlock) );
   block->next = NULL;
   block->instr = instr;
   block->nInstr = 0;

   // Estado inicial dos descritores
   block->addressDescriptorSize = Function_nLocals( function ) + Function_nTemps( function );
   block->addressDescriptor = (Variable**) malloc( block->addressDescriptorSize * sizeof( Variable* ) );
   for ( int i = 0 ; i < 6 ; i++ )
      block->registerDescriptor[i] = NULL;
   for ( int i = 0 ; i < block->addressDescriptorSize ; i++ )
      block->addressDescriptor[i] = NULL;

   while ( instr )
   {
      (block->nInstr)++;
      if ( instr->next == NULL ||
           instr->next->op == OP_LABEL ||
           instr->op == OP_GOTO ||
           instr->op == OP_IF ||
           instr->op == OP_IF_FALSE )
      {
         block->next = Block_generateBlocks( instr->next, function );
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



