/**
 * @file    sym.h
 * @author  lhpelosi
 */

#ifndef SYM_H
#define SYM_H

// Forward declaration
typedef struct ast Ast;



typedef struct symbol Symbol;
typedef struct scope Scope;
typedef struct symbolTable SymbolTable;
typedef enum dType DataType;

enum dType
{
   TYPE_INT,
   TYPE_BOOL,
   TYPE_CHAR,
   TYPE_VOID,

   TYPE_UNDEFINED
};

struct symbol
{
   char* name;
   DataType type;
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

