/**
 * @file    sym.c
 * @author  lhpelosi
 */

#include "sym.h"

#include <stdlib.h>

static SymbolTable* Sym_newSymbolTable();
static void Sym_delete( SymbolTable* table );
static void Sym_deleteScope( Scope* scope );
static void Sym_deleteSymbol( Symbol* symbol );
static Symbol* Sym_addSymbol( SymbolTable* table, char* name, SymbolType type, int nReferences, int line );
static Symbol* Sym_getSymbol( SymbolTable* table, const char* name, int isScopeSearch );
static void Sym_beginScope( SymbolTable* table );
static void Sym_endScope( SymbolTable* table );



int Sym_annotate( Ast* program )
{
   return 0;
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



Symbol* Sym_addSymbol( SymbolTable* table, char* name, SymbolType type, int nReferences, int line )
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

   return newSymbol;
}



Symbol* Sym_getSymbol( SymbolTable* table, const char* name, int isScopeSearch )
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



static void Sym_beginScope( SymbolTable* table )
{
   Scope* newScope = (Scope*) malloc( sizeof( Scope ) );
   newScope->symbolList = NULL;
   newScope->nextScope = table->innerScope;
   table->innerScope = newScope;
}



static void Sym_endScope( SymbolTable* table )
{
   Scope* scope = table->innerScope;
   table->innerScope = table->innerScope->nextScope;
   Sym_deleteScope( scope );
}



