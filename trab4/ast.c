/**
 * @file    ast.c
 * @author  lhpelosi
 */

#include "ast.h"

#include <stdlib.h>
#include <stdio.h>
#include "y.tab.h"
#include "sym.h"



static void Ast_printAux( Ast* ast, int ntab );
static Ast* newAstNode();
static char* typeTable( AstType type );



/*** Public ***/

Ast* Ast_new( AstType type, unsigned int line )
{
   Ast* node = newAstNode();
   node->type = type;
   node->line = line;

   return node;
}



Ast* Ast_newFromTokenIv( AstType type, int ivalue, unsigned int line )
{
   Ast* node = Ast_new( type, line );
   node->ival = ivalue;
   return node;
}



Ast* Ast_newFromTokenSv( AstType type, char* svalue, unsigned int line )
{
   Ast* node = Ast_new( type, line );
   node->sval = svalue;
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
   int ntabCurr = ntab;
   Ast* childCurr = NULL;

   if ( ast == NULL ) return;

   // Identacao
   while ( ntabCurr-- ) fprintf( stdout, "   " );

   // Imprime as infos do no
   fprintf( stdout, "%s", typeTable( ast->type ) );
   switch ( ast->type )
   { 
      case AST_VALSTRING:
      case AST_ID:
         fprintf( stdout, " [%s]", ast->sval );
         break;
      case AST_VALINT:
         fprintf( stdout, " [%d]", ast->ival );
         break;
      defaut:
         break;
   }

   // Imprime o tipo de dados do no
   if ( ast->dataType != TYPE_UNDEFINED )
   {
      const char* typeStr;
      int iReferences;
      fprintf( stdout, " : " );

      iReferences = ast->nReferences;
      while ( iReferences > 0 )
      {
         fprintf( stdout, "[]" );
         iReferences--;
      }

      switch ( ast->dataType )
      {
         case TYPE_INT:
            typeStr = "int";
            break;
         case TYPE_BOOL:
            typeStr = "bool";
            break;
         case TYPE_CHAR:
            typeStr = "char";
            break;
         case TYPE_VOID:
            typeStr = "void";
            break;
         default:
            typeStr = "";
            break;
      }
      fprintf( stdout, "%s", typeStr );
   }

   fprintf( stdout, " @%d", ast->line );

   // Iprime as infos dos filhos
   childCurr = ast->firstChild;
   if ( childCurr )
   {
      fprintf( stdout, " {\n" );

      do
      {
         Ast_printAux( childCurr, ntab+1 );
      } while ( childCurr = childCurr->nextSibling );

      ntabCurr = ntab;
      while ( ntabCurr-- ) fprintf( stdout, "   " );
      fprintf( stdout, "}" );
   }

   fprintf( stdout, "\n" );
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

   node->dataType = TYPE_UNDEFINED;
   node->nReferences = 0;

   node->parent = NULL;
   node->firstChild = NULL;
   node->lastChild = NULL;
   node->nextSibling = NULL;
   node->prevSibling = NULL;

   return node;
}



static char* typeTable( AstType type )
{
   switch ( type )
   {
      case AST_PROGRAM : return "PROGRAM";
      case AST_FUN : return "FUN";
      case AST_BLOCK : return "BLOCK";
      case AST_PARAMS : return "PARAMS";
      case AST_DECLVAR : return "DECLVAR";
      case AST_VAR : return "VAR";
      case AST_CMD_IF : return "CMD_IF";
      case AST_CMD_WHILE : return "CMD_WHILE";
      case AST_CMD_ATRIB : return "CMD_ATRIB";
      case AST_CMD_RETURN : return "CMD_RETURN";
      case AST_CALL : return "CALL";
      case AST_ARRAY : return "ARRAY";
      case AST_INT : return "INT";
      case AST_BOOL : return "BOOL";
      case AST_CHAR : return "CHAR";
      case AST_STRING : return "STRING";
      case AST_EXP_OR : return "EXP_OR";
      case AST_EXP_AND : return "EXP_AND";
      case AST_EXP_EQ : return "EXP_EQ";
      case AST_EXP_UNEQ : return "EXP_UNEQ";
      case AST_EXP_L : return "EXP_L";
      case AST_EXP_G : return "EXP_G";
      case AST_EXP_LEQ : return "EXP_LEQ";
      case AST_EXP_GEQ : return "EXP_GEQ";
      case AST_EXP_ADD : return "EXP_ADD";
      case AST_EXP_SUB : return "EXP_SUB";
      case AST_EXP_MULT : return "EXP_MULT";
      case AST_EXP_DIV : return "EXP_DIV";
      case AST_EXP_NEW : return "EXP_NEW";
      case AST_EXP_NOT : return "EXP_NOT";
      case AST_EXP_NEG : return "EXP_NEG";
      case AST_TRUE : return "TRUE";
      case AST_FALSE : return "FALSE";
      case AST_VALSTRING : return "VALSTRING";
      case AST_VALINT : return "VALINT";
      case AST_ID : return "ID";
   }
   return "!";
}



