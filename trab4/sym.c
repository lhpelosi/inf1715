/**
 * @file    sym.c
 * @author  lhpelosi
 */

#include "sym.h"

#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

static SymbolTable* Sym_newSymbolTable();
static void Sym_delete( SymbolTable* table );
static void Sym_deleteScope( Scope* scope );
static void Sym_deleteSymbol( Symbol* symbol );
static void Sym_addSymbol( SymbolTable* table, char* name, DataType type, int nReferences, int line );
static Symbol* Sym_getSymbol( SymbolTable* table, char* name, int isScopeSearch );
static void Sym_beginScope( SymbolTable* table );
static void Sym_endScope( SymbolTable* table );

static int Sym_visitProgram( SymbolTable* table, Ast* program );
static int Sym_visitFun( SymbolTable* table, Ast* fun );
static int Sym_visitDeclvar( SymbolTable* table, Ast* declvar );
static int Sym_visitBlock( SymbolTable* table, Ast* block );
static int Sym_visitVar( SymbolTable* table, Ast* var );
static int Sym_visitCmdIf( SymbolTable* table, Ast* cmdIf );
static int Sym_visitCmdWhile( SymbolTable* table, Ast* cmdWhile );
static int Sym_visitCmdAtrib( SymbolTable* table, Ast* cmdAtrib );
static int Sym_visitCmdReturn( SymbolTable* table, Ast* cmdReturn );
static int Sym_visitCall( SymbolTable* table, Ast* call );
static int Sym_visitExp( SymbolTable* table, Ast* exp );

static int Sym_checkDataType( Ast* node, DataType type, int nReferences );
static int Sym_matchDataType( Ast* node1, Ast* node2 );
static int Sym_getDataType( Ast* typeNode, DataType* dataType );
static void Sym_fail( const char* message, char* name, Ast* node );



int Sym_annotate( Ast* program )
{
   SymbolTable* table = Sym_newSymbolTable();
   return Sym_visitProgram( table, program );
}



SymbolTable* Sym_newSymbolTable()
{
   SymbolTable* table = (SymbolTable*) malloc( sizeof( SymbolTable ) );
   table->innerScope = (Scope*) malloc( sizeof( Scope ) );
   table->innerScope->symbolList = NULL;
   table->innerScope->nextScope = NULL;
   return table;
}



void Sym_delete( SymbolTable* table )
{
   // Destroi somente a tabela, nao os simbolos
   if ( table == NULL ) return;
   Sym_deleteScope( table->innerScope );
   free( table );
}



void Sym_deleteScope( Scope* scope )
{
   if ( scope == NULL ) return;
   free( scope );
}



void Sym_deleteSymbol( Symbol* symbol )
{
   free( symbol->name );
   free( symbol );
}



void Sym_addSymbol( SymbolTable* table, char* name, DataType type, int nReferences, int line )
{
   // Inicializa
   Symbol* newSymbol = (Symbol*) malloc( sizeof( Symbol ) );
   newSymbol->name = name;
   newSymbol->type = type;
   newSymbol->nReferences = nReferences;
   newSymbol->line = line;

   // Acrescenta na tabela
   newSymbol->nextSymbol = table->innerScope->symbolList;
   table->innerScope->symbolList = newSymbol;
}



Symbol* Sym_getSymbol( SymbolTable* table, char* name, int isScopeSearch )
{
   Scope* scope;
   Symbol* symbol;

   if ( table == NULL ) return NULL;

   for ( scope = table->innerScope ; scope != NULL ; scope = scope->nextScope )
   {
      for ( symbol = scope->symbolList ; symbol != NULL ; symbol = symbol->nextSymbol )
      {
         if ( strcmp( symbol->name, name ) == 0 ) return symbol;
      }

      // Caso esteja procurando apenas no escopo interno, pode parar
      if ( isScopeSearch ) return NULL;
   }

   return NULL;
}



void Sym_beginScope( SymbolTable* table )
{
   Scope* newScope = (Scope*) malloc( sizeof( Scope ) );
   newScope->symbolList = NULL;
   newScope->nextScope = table->innerScope;
   table->innerScope = newScope;
}



void Sym_endScope( SymbolTable* table )
{
   Scope* scope = table->innerScope;
   table->innerScope = table->innerScope->nextScope;
   Sym_deleteScope( scope );
}



int Sym_visitProgram( SymbolTable* table, Ast* program )
{
   Ast* child;
   int errors = 0;
   for ( child = program->firstChild ; child != NULL ; child = child->nextSibling )
   {
      if ( child->type == AST_FUN )
      {
         errors += Sym_visitFun( table, child );
      }
      else if ( child->type == AST_DECLVAR )
      {
         errors += Sym_visitDeclvar( table, child );
      }
   }
   return errors;
}



int Sym_visitFun( SymbolTable* table, Ast* fun )
{
   int errors = 0;
   char* name = fun->firstChild->sval;
   // Verifica se o simbolo ja existe no escopo
   Symbol* symbol = Sym_getSymbol( table, name, 1 );
   if ( symbol != NULL )
   {
      Sym_fail( "Simbolo com esse nome ja foi declarado anteriormente.", name, fun );
      return 1;
   }
   else
   {
      Ast* typeNode = fun->firstChild->nextSibling->nextSibling->nextSibling;
      int nReferences = 0;
      DataType symbolType;
      if ( typeNode == NULL )
      {
         symbolType = TYPE_VOID;
      }
      else
      {
         nReferences = Sym_getDataType( typeNode, &symbolType );
      }

      Sym_addSymbol( table, name, symbolType, nReferences, fun->line );
      fun->dataType = symbolType;
      fun->nReferences = nReferences;

      Sym_beginScope( table );
      // Visita os parametros
      Ast* parameter = fun->firstChild->nextSibling->firstChild;
      while ( parameter != NULL )
      {
         errors += Sym_visitDeclvar( table, parameter );
         parameter = parameter->nextSibling;
      }
      // Visita o bloco
      errors += Sym_visitBlock( table, fun->firstChild->nextSibling->nextSibling );
      Sym_endScope( table );

      return errors;
   }
}



int Sym_visitDeclvar( SymbolTable* table, Ast* declvar )
{
   char* name = declvar->firstChild->sval;
   // Verifica se o simbolo ja existe no escopo
   Symbol* symbol = Sym_getSymbol( table, name, 1 );
   if ( symbol != NULL )
   {
      Sym_fail( "Simbolo com esse nome ja foi declarado anteriormente.", name, declvar );
      return 1;
   }
   else
   {
      DataType dataType;
      Ast* typeNode = declvar->firstChild->nextSibling;
      int nReferences = Sym_getDataType( typeNode, &dataType );

      Sym_addSymbol( table, name, dataType, nReferences, declvar->line );
      declvar->dataType = dataType;
      declvar->nReferences = nReferences;
      return 0;
   }
}



int Sym_visitBlock( SymbolTable* table, Ast* block )
{
   int errors = 0;
   Ast* node = block->firstChild;
   while ( node != NULL )
   {
      switch ( node->type )
      {
         case AST_DECLVAR :
            errors += Sym_visitDeclvar( table, node );
            break;
         case AST_CMD_IF :
            errors += Sym_visitCmdIf( table, node );
            break;
         case AST_CMD_WHILE :
            errors += Sym_visitCmdWhile( table, node );
            break;
         case AST_CMD_ATRIB :
            errors += Sym_visitCmdAtrib( table, node );
            break;
         case AST_CMD_RETURN :
            errors += Sym_visitCmdReturn( table, node );
            break;
         case AST_CALL :
            errors += Sym_visitCall( table, node );
            break;
         default:
            break;
      }
      node = node->nextSibling;
   }
   return errors;
}



int Sym_visitVar( SymbolTable* table, Ast* var )
{
   int errors = 0;
   char* name = var->firstChild->sval;
   Symbol* symbol = Sym_getSymbol( table, name, 0 );
   if ( symbol == NULL )
   {
      Sym_fail( "Simbolo nao declarado.", name, var->firstChild );
      return 1;
   }
   var->dataType = symbol->type;
   var->nReferences = symbol->nReferences;

   Ast* indexer = var->firstChild->nextSibling;
   while( indexer != NULL )
   {
      errors += Sym_visitExp( table, indexer );
      errors += Sym_checkDataType( indexer, TYPE_INT, 0 );
      (var->nReferences)--; // Pois a indexacao faz 'perder' uma referencia
      indexer = indexer->nextSibling;
   }

   return errors;
}



int Sym_visitCmdIf( SymbolTable* table, Ast* cmdIf )
{
   int errors = 0;
   Ast* node = cmdIf->firstChild;
   while ( node != NULL )
   {
      if ( node->type == AST_BLOCK )
      {
         Sym_beginScope( table );
         errors += Sym_visitBlock( table, node );
         Sym_endScope( table );
      }
      else
      {
         errors += Sym_visitExp( table, node );
         errors += Sym_checkDataType( node, TYPE_BOOL, 0 );
      }
      node = node->nextSibling;
   }
   return errors;
}



int Sym_visitCmdWhile( SymbolTable* table, Ast* cmdWhile )
{
   int errors = 0;
   Ast* exp = cmdWhile->firstChild;
   Ast* block = cmdWhile->firstChild->nextSibling;

   errors += Sym_visitExp( table, exp );
   errors += Sym_checkDataType( exp, TYPE_BOOL, 0 );

   Sym_beginScope( table );
   errors += Sym_visitBlock( table, block );
   Sym_endScope( table );

   return errors;
}



int Sym_visitCmdAtrib( SymbolTable* table, Ast* cmdAtrib )
{
   int errors = 0;
   Ast* left = cmdAtrib->firstChild;
   Ast* right = cmdAtrib->firstChild->nextSibling;

   errors += Sym_visitVar( table, left );
   errors += Sym_visitExp( table, right );

   errors += Sym_matchDataType( left, right );

   return errors;
}



int Sym_visitCmdReturn( SymbolTable* table, Ast* cmdReturn )
{
   int errors = 0;

   Ast* parentFunction = cmdReturn;
   while ( parentFunction->type != AST_FUN )
      parentFunction = parentFunction->parent;

   // Return simples
   if ( cmdReturn->firstChild == NULL )
   {
      cmdReturn->dataType = TYPE_VOID;
   }
   // Return com expressao
   else
   {
      errors += Sym_visitExp( table, cmdReturn->firstChild );
      cmdReturn->dataType = cmdReturn->firstChild->dataType;
      cmdReturn->nReferences = cmdReturn->firstChild->nReferences;
   }

   errors += Sym_matchDataType( cmdReturn, parentFunction );
   return errors;
}



int Sym_visitCall( SymbolTable* table, Ast* call )
{
   int errors = 0;
   char* name = call->firstChild->sval;
   Symbol* symbol = Sym_getSymbol( table, name, 0 );
   if ( symbol == NULL )
   {
      Sym_fail( "Simbolo nao declarado.", name, call );
      return 1;
   }
   call->dataType = symbol->type;
   call->nReferences = symbol->nReferences;

   Ast* parameter = call->firstChild->nextSibling;
   while( parameter != NULL )
   {
      errors += Sym_visitExp( table, parameter );
      parameter = parameter->nextSibling;
   }
   return errors;
}



int Sym_visitExp( SymbolTable* table, Ast* exp )
{
   int errors = 0;
   switch ( exp->type )
   {
      case AST_VALINT:
         exp->dataType = TYPE_INT;
         break;

      case AST_VALSTRING:
         // Promocao de []char para []int na expressao
         exp->dataType = TYPE_INT;
         exp->nReferences = 1;
         break;

      case AST_TRUE:
      case AST_FALSE:
         exp->dataType = TYPE_BOOL;
         break;

      case AST_EXP_NOT:
         exp->dataType = TYPE_BOOL;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_checkDataType( exp->firstChild, TYPE_BOOL, 0 );
         break;

      case AST_EXP_NEG:
         exp->dataType = TYPE_INT;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_checkDataType( exp->firstChild, TYPE_INT, 0 );
         break;

      case AST_EXP_OR:
      case AST_EXP_AND:
         exp->dataType = TYPE_BOOL;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_visitExp( table, exp->firstChild->nextSibling );
         errors += Sym_checkDataType( exp->firstChild, TYPE_BOOL, 0 );
         errors += Sym_checkDataType( exp->firstChild->nextSibling, TYPE_BOOL, 0 );
         break;

      case AST_EXP_EQ:
      case AST_EXP_UNEQ:
         exp->dataType = TYPE_BOOL;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_visitExp( table, exp->firstChild->nextSibling );
         errors += Sym_matchDataType( exp->firstChild, exp->firstChild->nextSibling );
         break;

      case AST_EXP_L:
      case AST_EXP_G:
      case AST_EXP_LEQ:
      case AST_EXP_GEQ:
         exp->dataType = TYPE_BOOL;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_visitExp( table, exp->firstChild->nextSibling );
         errors += Sym_checkDataType( exp->firstChild, TYPE_INT, 0 );
         errors += Sym_checkDataType( exp->firstChild->nextSibling, TYPE_INT, 0 );
         break;

      case AST_EXP_ADD:
      case AST_EXP_SUB:
      case AST_EXP_MULT:
      case AST_EXP_DIV:
         exp->dataType = TYPE_INT;
         errors += Sym_visitExp( table, exp->firstChild );
         errors += Sym_visitExp( table, exp->firstChild->nextSibling );
         errors += Sym_checkDataType( exp->firstChild, TYPE_INT, 0 );
         errors += Sym_checkDataType( exp->firstChild->nextSibling, TYPE_INT, 0 );
         break;

      case AST_EXP_NEW:
         errors += Sym_visitExp( table, exp->firstChild );
         exp->nReferences = Sym_getDataType( exp->firstChild->nextSibling, &(exp->dataType) );
         (exp->nReferences)++; // Pois o proprio new adiciona uma referencia
         break;

      case AST_VAR:
         errors += Sym_visitVar( table, exp );
         break;

      case AST_CALL:
         errors += Sym_visitCall( table, exp );
         break;

      default:
         break;
   }
   return errors;
}



int Sym_checkDataType( Ast* node, DataType type, int nReferences )
{
   if ( node->dataType != type || node->nReferences != nReferences )
   {
      fprintf( stderr, "Linha %d: Tipo incompativel.\n", node->line );
      return 1;
   }
   return 0;
}



int Sym_matchDataType( Ast* node1, Ast* node2 )
{
   if ( node1->dataType != node2->dataType || node1->nReferences != node2->nReferences )
   {
      fprintf( stderr, "Linha %d: Tipo incompativel.\n", node1->line );
      return 1;
   }
   return 0;
}



// O retorno eh a contagem de referencias e o parametro dataType retorna o tipo de dado base por referencia.
int Sym_getDataType( Ast* typeNode, DataType* dataType )
{
   int nReferences = 0;
   while ( typeNode->type == AST_ARRAY )
   {
      nReferences++;
      typeNode = typeNode->firstChild;
   }
   switch ( typeNode->type )
   {
      case AST_INT :
         *dataType = TYPE_INT;
         break;
      case AST_BOOL :
         *dataType = TYPE_BOOL;
         break;
      case AST_CHAR :
         *dataType = TYPE_INT; // Conversao de char pra int
         break;
      case AST_STRING :
         *dataType = TYPE_INT; // Conversao de char pra int
         nReferences++;
         break;
      default :
         break;
   }
   return nReferences;
}



void Sym_fail( const char* message, char* name, Ast* node )
{
   fprintf( stderr, "Linha %d: \'%s\'. %s\n", node->line, name, message );
}



