/**
 * @file    ast.c
 * @author  lhpelosi
 */

#include "ast.h"

#include <stdlib.h>
#include <stdio.h>
#include "y.tab.h"

struct ast
{
   AstType type;
   unsigned int line;
   int ival;
   char* sval;

   Ast* parent;
   Ast* firstChild;
   Ast* lastChild;
   Ast* nextSibling;
   Ast* prevSibling;
};

static void Ast_printAux( Ast* ast, int ntab );
static Ast* newAstNode();



/*** Public ***/

Ast* Ast_new( AstType type, unsigned int line )
{
   Ast* node = newAstNode();
   node->type = type;
   node->line = line;

   return node;
}



Ast* Ast_newFromToken( unsigned int token, unsigned int line )
{
   Ast* node = newAstNode();
   node->line = line;

   switch ( token )
   {
      case TK_VALSTRING:
         node->type = AST_VALSTRING;
         //node->sval = yylval.sval;
         break;

      case TK_VALINT:
         node->type = AST_VALINT;
         //node->ival = yylval.ival;
         break;

      case TK_ID:
         node->type = AST_ID;
         //node->sval = yylval.sval;
         break;
   }

   return node;
}



void Ast_addChild( Ast* parentNode, Ast* childNode )
{
   if ( parentNode == NULL || childNode == NULL ) return;

   childNode->parent = parentNode;
   childNode->prevSibling = parentNode->lastChild;

   if ( parentNode->lastChild != NULL )
      parentNode->lastChild->nextSibling = childNode;

   if ( parentNode->firstChild == NULL )
      parentNode->firstChild = childNode;

   parentNode->lastChild = childNode;
}



void Ast_addChildren( Ast* parentNode, Ast* childrenList )
{
   if ( parentNode == NULL || childrenList == NULL ) return;

   Ast* firstChildFromList;
   Ast* lastChildFromList;

   firstChildFromList = childrenList;
   while ( firstChildFromList->prevSibling != NULL )
      firstChildFromList = firstChildFromList->prevSibling;

   firstChildFromList->prevSibling = parentNode->lastChild;
   if ( parentNode->lastChild != NULL )
      parentNode->lastChild->nextSibling = firstChildFromList;

   if ( parentNode->firstChild == NULL )
      parentNode->firstChild = firstChildFromList;

   childrenList = firstChildFromList;
   while ( childrenList != NULL )
   {
      childrenList->parent = parentNode;

      lastChildFromList = childrenList;
      childrenList = childrenList->nextSibling;
   }
   parentNode->lastChild = lastChildFromList;
}



Ast* Ast_prependSibling( Ast* firstList, Ast* secondList )
{
   if ( firstList == NULL ) return secondList;
   if ( secondList == NULL ) return firstList;

   Ast* lastFromFirstList = firstList;
   Ast* firstFromSecondList = secondList;

   while ( lastFromFirstList->nextSibling != NULL )
      lastFromFirstList = lastFromFirstList->nextSibling;

   while ( firstFromSecondList->prevSibling != NULL )
      firstFromSecondList = firstFromSecondList->prevSibling;
   
   lastFromFirstList->nextSibling = firstFromSecondList;
   firstFromSecondList->prevSibling = lastFromFirstList;

   return firstList;
}



void Ast_print( Ast* ast )
{
   Ast_printAux( ast, 0 );
}



/*** Private ***/

void Ast_printAux( Ast* ast, int ntab )
{
   return;
}



Ast* newAstNode()
{
   Ast* node = (Ast*) malloc( sizeof( Ast ) );
   if ( node == NULL )
   {
      fprintf( stderr, "Memoria insuficiente!\n" );
      exit( -1 );
   }
   
   node->type = AST_INVALID;
   node->line = 0;
   node->ival = 0;
   node->sval = NULL;

   node->parent = NULL;
   node->firstChild = NULL;
   node->lastChild = NULL;
   node->nextSibling = NULL;
   node->prevSibling = NULL;

   return node;
}



int main(void)
{
   return 0;
}



