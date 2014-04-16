/**
 * @file    ast.h
 * @author  lhpelosi
 */

#ifndef AST_H
#define AST_H

// Tipo basico de no da arvore sintatica abstrata
typedef struct ast Ast;

// Tipos de no
typedef enum astType
{
   AST_VALSTRING,
   AST_VALINT,
   AST_ID,
   AST_INVALID
} AstType;

Ast* Ast_new( AstType type, unsigned int line );
Ast* Ast_newFromToken( unsigned int token, unsigned int line );
void Ast_addChild( Ast* parentNode, Ast* childNode );
void Ast_addChildren( Ast* parentNode, Ast* childrenList );
Ast* Ast_prependSibling( Ast* firstList, Ast* secondList );
void Ast_print( Ast* ast );

#endif
