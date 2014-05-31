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
   // AST_PROGRAM possui numero indeterminado de filhos que podem ser AST_FUN ou AST_DECLVAR.
   AST_PROGRAM,

   // AST_FUN possui quatro filhos nessa ordem: AST_ID, AST_PARAMS, AST_BLOCK, e por ultimo um no do conjunto de tipos de dados (se for NULL eh void)
   AST_FUN,

   // AST_BLOCK possui numero indeterminado de filhos do tipo AST_DECLVAR, seguido de indeterminados nos do conjunto de comandos.
   AST_BLOCK,

   // AST_PARAMS possui numero indeterminado de filhos do tipo AST_DECLVAR.
   AST_PARAMS,

   // AST_DECLVAR possui 1 filho do tipo AST_ID seguido de 1 filho do conjunto de tipos de dados.
   AST_DECLVAR,

   // AST_VAR possui 1 filho do tipo AST_ID seguido de numero indeterminado de filhos dos conjuntos de expressoes.
   AST_VAR,



   // Conjunto de comandos (inclui AST_CALL)

   // AST_CMD_IF possui indeterminado numero (pelo menos 1) de pares de filhos ( 1 filho do conjunto de expressoes seguido de 1 filho AST_BLOCK ) seguido opcionalmente de 1 filho AST_BLOCK.
   AST_CMD_IF,

   // AST_CMD_WHILE possui 1 filho de tipo do conjunto de expressoes seguido de 1 filho de tipo AST_BLOCK.
   AST_CMD_WHILE,

   // AST_CMD_ATRIB possui 1 filho do tipo AST_VAR, seguido de 1 filho de tipo do conjunto de expressoes.
   AST_CMD_ATRIB,

   // AST_CMD_RETURN possui opcionalmente um filho de tipo do conjunto de expressoes.
   AST_CMD_RETURN,

   // AST_CALL possui 1 filho do tipo AST_ID seguido de numero indeterminado de filhos dos conjuntos de expressoes.
   AST_CALL,



   // Conjunto de tipos de dados

   // AST_ARRAY deve ter um filho de qualquer tipo de dado, inclusive AST_ARRAY.
   // Todos os demais sao folhas.
   AST_ARRAY,
   AST_INT,
   AST_BOOL,
   AST_CHAR,
   AST_STRING,



   // Conjuntos de expressoes

   // Conjunto de expressoes de 2 filhos
   // Os filhos podem ser qualquer outra expressao de 2 filhos, de 1 filho ou expressoes simples.
   AST_EXP_OR,
   AST_EXP_AND,
   AST_EXP_EQ,
   AST_EXP_UNEQ,
   AST_EXP_L,
   AST_EXP_G,
   AST_EXP_LEQ,
   AST_EXP_GEQ,
   AST_EXP_ADD,
   AST_EXP_SUB,
   AST_EXP_MULT,
   AST_EXP_DIV,
   // AST_EXP_NEW possui 1 filho de tipo do conjunto de expressoes, seguido de filho do conjunto de tipos de dados.
   AST_EXP_NEW,

   // Conjunto de expressoes de 1 filho
   // Os filhos podem ser qualquer outra expressao de 2 filhos, de 1 filho ou expressoes simples.
   AST_EXP_NOT,
   AST_EXP_NEG,

   // Conjunto de expressoes simples. (Tambem podem ser expressoes simples as AST_VAR e AST_CALL)
   // Sao todos folhas.
   AST_TRUE,
   AST_FALSE,
   AST_VALSTRING, // Possui sval
   AST_VALINT, // Possui ival
   AST_ID, // Possui sval



   AST_INVALID

} AstType;



/**
 * Cria um novo no
 * @param type Indica o tipo de estrutura representado pelo no
 * @param line A linha em que essa estrutura se encontra no arquivo de entrada
 * @return O no gerado
 */
Ast* Ast_new( AstType type, unsigned int line );

/**
 * Cria um novo no com tipo semantico de inteiro
 * @param type Indica o tipo de estrutura representado pelo no
 * @param ivalue O valor de inteiro que ele guarda
 * @param line A linha em que essa estrutura se encontra no arquivo de entrada
 * @return O no gerado
 */
Ast* Ast_newFromTokenIv( AstType type, int ivalue, unsigned int line );

/**
 * Cria um novo no com tipo semantico de string
 * @param type Indica o tipo de estrutura representado pelo no
 * @param ivalue O valor de string que ele guarda
 * @param line A linha em que essa estrutura se encontra no arquivo de entrada
 * @return O no gerado
 */
Ast* Ast_newFromTokenSv( AstType type, char* svalue, unsigned int line );

/**
 * Adiciona um unico no como filho de outro
 * @param parentNode O no que sera o pai
 * @param childNode O no que sera o filho
 */
void Ast_addChild( Ast* parentNode, Ast* childNode );

/**
 * Adiciona uma lista de nos como filho de um no
 * @param parentNode O no que sera o pai
 * @param childrenList A lista de nos que serao filhos dele
 */
void Ast_addChildren( Ast* parentNode, Ast* childrenList );

/**
 * Junta duas listas de nos (podendo ter inclusive 0 ou 1 elemento) como irmaos
 * @param firstList Primeira lista de nos
 * @param secondList Segunda lista de nos
 * @return A lista resultante da uniao
 */
Ast* Ast_prependSibling( Ast* firstList, Ast* secondList );

/**
 * Imprime uma arvore sintatica na stdout
 * @param ast A arvore sintatica abstrata do programa a ser impresso
 */
void Ast_print( Ast* ast );

#endif
