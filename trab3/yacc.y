%{
/**
 * @file    yacc.y
 * @author  lhpelosi
 */

# include <stdio.h>
# include "ast.h"

extern FILE * yyin;
int yydebug=1;

// Identifica erros detectados no lexico ( 0 == sem erros )
unsigned int _lexicalError = 0;

// Identifica erros detectados de sintaxe ( 0 == sem erros )
unsigned int _syntaxError = 0;

// Numero da linha corrente de leitura do arquivo
extern unsigned int _line;

%}

%union
{
   int ival;
   char * sval;
   Ast* node;
}

// Tipos de token

%token TK_NL

%token TK_IF
%token TK_ELSE
%token TK_END
%token TK_WHILE
%token TK_LOOP
%token TK_FUN
%token TK_RETURN
%token TK_NEW
%token TK_STRING
%token TK_INT
%token TK_CHAR
%token TK_BOOL
%token TK_TRUE
%token TK_FALSE
%token TK_AND
%token TK_OR
%token TK_NOT
%token TK_GEQUALS
%token TK_LEQUALS
%token TK_UNEQUALS

%token TK_VALSTRING
%token TK_VALINT
%token TK_ID

%start programa

%%
programa       : nl programaaux              { $$.node = Ast_new( AST_PROGRAM, 1 );
                                               Ast_addChildren( $$.node, $2.node ); }
               | programaaux                 { $$.node = Ast_new( AST_PROGRAM, 1 );
                                               Ast_addChildren( $$.node, $1.node ); }
               ;
programaaux    : decl                        { $$ = $1; }
               | programaaux decl            { $$.node = Ast_prependSibling( $1.node, $2.node ); }
               ;
decl           : funcao                      { $$ = $1; }
               | global                      { $$ = $1; }
               ;
nl             : TK_NL                       {}
               | nl TK_NL                    {}
               ;
global         : declvar nl                  { $$ = $1; }
               ;
funcao         : TK_FUN TK_ID '(' params ')' funtipo nl bloco TK_END nl
                                             { $$.node = Ast_new( AST_FUN, _line );
                                               Ast* parameters = Ast_new( AST_PARAMS, _line );
                                               Ast_addChildren( parameters, $4.node );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $8.node );
                                               Ast_addChild( $$.node, Ast_newFromToken($2,_line) );
                                               Ast_addChild( $$.node, parameters );
                                               Ast_addChild( $$.node, block );
                                               Ast_addChild( $$.node, $6.node ); }
               ;
funtipo        : /* vazio */                 { $$.node = NULL; }
               | ':' tipo                    { $$ = $2; }
               ;
bloco          : /* vazio */                 { $$.node = NULL; }
               | listadeclvar                { $$ = $1; }
               | listacomando                { $$ = $1; }
               | listadeclvar listacomando   { $$.node = Ast_prependSibling( $1.node, $2.node ); }
               ;
listadeclvar   : declvar nl                  { $$ = $1; }
               | listadeclvar declvar nl     { $$.node = Ast_prependSibling( $1.node, $2.node ); }
               ;
listacomando   : comando nl                  { $$ = $1; }
               | listacomando comando nl     { $$.node = Ast_prependSibling( $1.node, $2.node ); }
               ;
params         : /* vazio */                 { $$.node = NULL; }
               | paramsaux                   { $$ = $1; }
               ;
paramsaux      : parametro                   { $$ = $1; }
               | paramsaux ',' parametro     { $$.node = Ast_prependSibling( $1.node, $3.node ); }
               ;
parametro      : TK_ID ':' tipo              { $$.node = Ast_new( AST_DECLVAR, _line );
                                               Ast_addChild( $$.node, Ast_newFromToken($1,_line) );
                                               Ast_addChild( $$.node, $3.node ); }
               ;
tipo           : tipobase                    { $$ = $1; }
               | '[' ']' tipo                { $$.node = Ast_new( AST_ARRAY, _line );
                                               Ast_addChild( $$.node, $3.node ); }
               ;
tipobase       : TK_INT                      { $$.node = Ast_new( AST_INT, _line ); }
               | TK_BOOL                     { $$.node = Ast_new( AST_BOOL, _line ); }
               | TK_CHAR                     { $$.node = Ast_new( AST_CHAR, _line ); }
               | TK_STRING                   { $$.node = Ast_new( AST_STRING, _line ); }
               ;
declvar        : TK_ID ':' tipo              { $$.node = Ast_new( AST_DECLVAR, _line );
                                               Ast_addChild( $$.node, Ast_newFromToken($1,_line) );
                                               Ast_addChild( $$.node, $3.node ); }
               ;
comando        : cmdif                       { $$ = $1; }
               | cmdwhile                    { $$ = $1; }
               | cmdatrib                    { $$ = $1; }
               | cmdreturn                   { $$ = $1; }
               | chamada                     { $$ = $1; }
               ;
cmdif          : TK_IF exp nl bloco cmdelseif cmdelse TK_END
                                             { $$.node = Ast_new( AST_CMD_IF, _line );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $4.node );
                                               Ast* temp = Ast_prependSibling( $2.node, block );
                                               Ast* temp = Ast_prependSibling( temp, $5.node );
                                               Ast* temp = Ast_prependSibling( temp, $6.node );
                                               Ast_addChildren( $$.node, temp ); }
               ;
cmdelseif      : /* vazio */                 { $$.node = NULL; }
               | cmdelseif TK_ELSE TK_IF exp nl bloco
                                             { Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $6.node );
                                               Ast* temp = Ast_prependSibling( $4.node, block );
                                               $$.node = Ast_prependSibling( $1.node, temp ); }
               ;
cmdelse        : /* vazio */                 { $$.node = NULL; }
               | TK_ELSE nl bloco            { $$ = $3; }
               ;
cmdwhile       : TK_WHILE exp nl bloco TK_LOOP
                                             { $$.node = Ast_new( AST_CMD_WHILE, _line );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $4.node );
                                               Ast_addChild( $$.node, $2.node );
                                               Ast_addChild( $$.node, block ); }
               ;
cmdatrib       : var '=' exp                 { $$.node = Ast_new( AST_CMD_ATRIB , _line );
                                               Ast* v = Ast_new( AST_VAR , _line );
                                               Ast_addChildren( v, $1.node );
                                               Ast_addChild( $$.node, v );
                                               Ast_addChild( $$.node, $3.node ); }
               ;
chamada        : TK_ID '(' listaexp ')'      { $$.node = Ast_new( AST_CALL , _line );
                                               Ast_addChild( $$.node, Ast_newFromToken($1,_line) );
                                               Ast_addChildren( $$.node, $3.node ); }
               ;
listaexp       : /* vazio */                 { $$.node = NULL; }
               | listaexpaux                 { $$ = $1; }
               ;
listaexpaux    : exp                         { $$ = $1; }
               | listaexpaux ',' exp         { $$.node = Ast_prependSibling( $1.node, $3.node ); }
               ;
cmdreturn      : TK_RETURN exp               { $$.node = Ast_new( AST_RETURN , _line );
                                               Ast_addChild( $$.node, $2.node ); }
               | TK_RETURN                   { $$.node = Ast_new( AST_RETURN , _line ); }
               ;
var            : TK_ID                       { $$ = $1; }
               | var '[' exp ']'             { $$.node = Ast_prependSibling( $1.node, $3.node ); }
               ;
exp      : exp_or                            { $$ = $1; }
         ;
exp_or   : exp_and                           { $$ = $1; }
         | exp_or TK_OR exp_and              { $$.node = Ast_new( AST_EXP_OR , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_and  : exp_eq                            { $$ = $1; }
         | exp_and TK_AND exp_eq             { $$.node = Ast_new( AST_EXP_AND , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_eq   : exp_rel                           { $$ = $1; }
         | exp_eq '=' exp_rel                { $$.node = Ast_new( AST_EXP_EQ , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_eq TK_UNEQUALS exp_rel        { $$.node = Ast_new( AST_EXP_UNEQ , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_rel  : exp_add                           { $$ = $1; }
         | exp_rel '<' exp_add               { $$.node = Ast_new( AST_EXP_L , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_rel '>' exp_add               { $$.node = Ast_new( AST_EXP_G , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_rel TK_LEQUALS exp_add        { $$.node = Ast_new( AST_EXP_LEQ , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_rel TK_GEQUALS exp_add        { $$.node = Ast_new( AST_EXP_GEQ , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_add  : exp_mult                          { $$ = $1; }
         | exp_add '+' exp_mult              { $$.node = Ast_new( AST_EXP_ADD , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_add '-' exp_mult              { $$.node = Ast_new( AST_EXP_SUB , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_mult : exp_base                          { $$ = $1; }
         | exp_mult '*' exp_base             { $$.node = Ast_new( AST_EXP_MULT , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         | exp_mult '/' exp_base             { $$.node = Ast_new( AST_EXP_DIV , _line );
                                               Ast_addChild( $$.node, $1.node );
                                               Ast_addChild( $$.node, $3.node ); }
         ;
exp_base : TK_VALINT                         { $$.node = Ast_newFromToken($1,_line); }
         | TK_VALSTRING                      { $$.node = Ast_newFromToken($1,_line); }
         | TK_TRUE                           { $$.node = Ast_new( AST_TRUE , _line ); }
         | TK_FALSE                          { $$.node = Ast_new( AST_FALSE , _line ); }
         | var                               { $$.node = Ast_new( AST_VAR , _line );
                                               Ast_addChildren( $$.node, $1.node ); }
         | TK_NEW '[' exp ']' tipo           { $$.node = Ast_new( AST_EXP_NEW , _line );
                                               Ast_addChild( $$.node, $3.node );
                                               Ast_addChild( $$.node, $5.node ); }
         | '(' exp ')'                       { $$ = $2; }
         | chamada                           { $$ = $1; }
         | TK_NOT exp_base                   { $$.node = Ast_new( AST_EXP_NOT , _line );
                                               Ast_addChild( $$.node, $2.node ); }
         | '-' exp_base                      { $$.node = Ast_new( AST_EXP_NEG , _line );
                                               Ast_addChild( $$.node, $2.node ); }
         ;
%%

yyerror( char *s )
{
   _syntaxError++;
   fprintf( stderr, "Linha %d: Erro de sintaxe.\n", _line );
}

int main( int argc, char * argv[] )
{
   if ( argc > 1 )
      yyin = fopen( argv[1] ,"r" );

   if ( yyin == NULL )
   {
      fprintf( stderr, "Arquivo nao encontrado!\n" );
      return -1;
   }

   yyparse();

   if ( _lexicalError > 0 || _syntaxError > 0 )
      return -1;
   else
   {
      fprintf( stdout, "Parsing realizado com sucesso!\n" );
      return 0;
   }
}

