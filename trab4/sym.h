/**
 * @file    sym.h
 * @author  lhpelosi
 */

#ifndef SYM_H
#define SYM_H

#include "ast.h"

typedef struct symbol Symbol;
typedef struct scope Scope;
typedef struct symbolTable SymbolTable;
typedef enum symbolType
{
   SYM_INT,
   SYM_BOOL,
   SYM_CHAR,
   SYM_FUN
} SymbolType;

struct symbol
{
   char* name;
   SymbolType type;
   int nReferences;
   int line;
   Symbol* nextSymbol;
};

struct scope
{
   Symbol* symbolList;
   Scope* nextScope;
};

struct symbolTable
{
   Scope* innerScope;
};

int Sym_annotate( Ast* program );

#endif

