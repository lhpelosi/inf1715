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

// Guarda a arvore sintatica abstrata do programa lido
Ast* _program = NULL;

// Numero da linha corrente de leitura do arquivo
extern unsigned int _line;

%}

%type <node> programa
%type <node> programaaux
%type <node> decl
%type <node> nl
%type <node> global
%type <node> funcao
%type <node> funtipo
%type <node> bloco
%type <node> listadeclvar
%type <node> listacomando
%type <node> params
%type <node> paramsaux
%type <node> parametro
%type <node> tipo
%type <node> tipobase
%type <node> declvar
%type <node> comando
%type <node> cmdif
%type <node> cmdelseif
%type <node> cmdelse
%type <node> cmdwhile
%type <node> cmdatrib
%type <node> chamada
%type <node> listaexp
%type <node> listaexpaux
%type <node> cmdreturn
%type <node> var
%type <node> exp
%type <node> exp_or
%type <node> exp_and
%type <node> exp_eq
%type <node> exp_rel
%type <node> exp_add
%type <node> exp_mult
%type <node> exp_base

%union
{
   int ival;
   char* sval;
   struct ast* node;
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

%token <sval> TK_VALSTRING
%token <ival> TK_VALINT
%token <sval> TK_ID

%start programa

%%
programa       : nl programaaux              { _program = Ast_new( AST_PROGRAM, 1 );
                                               Ast_addChildren( $$, $2 ); }
               | programaaux                 { _program = Ast_new( AST_PROGRAM, 1 );
                                               Ast_addChildren( $$, $1 ); }
               ;
programaaux    : decl                        { $$ = $1; }
               | programaaux decl            { $$ = Ast_prependSibling( $1, $2 ); }
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
                                             { $$ = Ast_new( AST_FUN, _line );
                                               Ast* parameters = Ast_new( AST_PARAMS, _line );
                                               Ast_addChildren( parameters, $4 );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $8 );
                                               Ast_addChild( $$, Ast_newFromTokenSv( AST_ID, $2, _line) );
                                               Ast_addChild( $$, parameters );
                                               Ast_addChild( $$, block );
                                               Ast_addChild( $$, $6 ); }
               ;
funtipo        : /* vazio */                 { $$ = NULL; }
               | ':' tipo                    { $$ = $2; }
               ;
bloco          : /* vazio */                 { $$ = NULL; }
               | listadeclvar                { $$ = $1; }
               | listacomando                { $$ = $1; }
               | listadeclvar listacomando   { $$ = Ast_prependSibling( $1, $2 ); }
               ;
listadeclvar   : declvar nl                  { $$ = $1; }
               | listadeclvar declvar nl     { $$ = Ast_prependSibling( $1, $2 ); }
               ;
listacomando   : comando nl                  { $$ = $1; }
               | listacomando comando nl     { $$ = Ast_prependSibling( $1, $2 ); }
               ;
params         : /* vazio */                 { $$ = NULL; }
               | paramsaux                   { $$ = $1; }
               ;
paramsaux      : parametro                   { $$ = $1; }
               | paramsaux ',' parametro     { $$ = Ast_prependSibling( $1, $3 ); }
               ;
parametro      : TK_ID ':' tipo              { $$ = Ast_new( AST_DECLVAR, _line );
                                               Ast_addChild( $$, Ast_newFromTokenSv( AST_ID, $1, _line ) );
                                               Ast_addChild( $$, $3 ); }
               ;
tipo           : tipobase                    { $$ = $1; }
               | '[' ']' tipo                { $$ = Ast_new( AST_ARRAY, _line );
                                               Ast_addChild( $$, $3 ); }
               ;
tipobase       : TK_INT                      { $$ = Ast_new( AST_INT, _line ); }
               | TK_BOOL                     { $$ = Ast_new( AST_BOOL, _line ); }
               | TK_CHAR                     { $$ = Ast_new( AST_CHAR, _line ); }
               | TK_STRING                   { $$ = Ast_new( AST_STRING, _line ); }
               ;
declvar        : TK_ID ':' tipo              { $$ = Ast_new( AST_DECLVAR, _line );
                                               Ast_addChild( $$, Ast_newFromTokenSv( AST_ID, $1, _line) );
                                               Ast_addChild( $$, $3 ); }
               ;
comando        : cmdif                       { $$ = $1; }
               | cmdwhile                    { $$ = $1; }
               | cmdatrib                    { $$ = $1; }
               | cmdreturn                   { $$ = $1; }
               | chamada                     { $$ = $1; }
               ;
cmdif          : TK_IF exp nl bloco cmdelseif cmdelse TK_END
                                             { $$ = Ast_new( AST_CMD_IF, _line );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $4 );
                                               Ast* temp = Ast_prependSibling( $2, block );
                                               temp = Ast_prependSibling( temp, $5 );
                                               temp = Ast_prependSibling( temp, $6 );
                                               Ast_addChildren( $$, temp ); }
               ;
cmdelseif      : /* vazio */                 { $$ = NULL; }
               | cmdelseif TK_ELSE TK_IF exp nl bloco
                                             { Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $6 );
                                               Ast* temp = Ast_prependSibling( $4, block );
                                               $$ = Ast_prependSibling( $1, temp ); }
               ;
cmdelse        : /* vazio */                 { $$ = NULL; }
               | TK_ELSE nl bloco            { $$ = $3; }
               ;
cmdwhile       : TK_WHILE exp nl bloco TK_LOOP
                                             { $$ = Ast_new( AST_CMD_WHILE, _line );
                                               Ast* block = Ast_new( AST_BLOCK, _line );
                                               Ast_addChildren( block, $4 );
                                               Ast_addChild( $$, $2 );
                                               Ast_addChild( $$, block ); }
               ;
cmdatrib       : var '=' exp                 { $$ = Ast_new( AST_CMD_ATRIB , _line );
                                               Ast* v = Ast_new( AST_VAR , _line );
                                               Ast_addChildren( v, $1 );
                                               Ast_addChild( $$, v );
                                               Ast_addChild( $$, $3 ); }
               ;
chamada        : TK_ID '(' listaexp ')'      { $$ = Ast_new( AST_CALL , _line );
                                               Ast_addChild( $$, Ast_newFromTokenSv( AST_ID, $1, _line ) );
                                               Ast_addChildren( $$, $3 ); }
               ;
listaexp       : /* vazio */                 { $$ = NULL; }
               | listaexpaux                 { $$ = $1; }
               ;
listaexpaux    : exp                         { $$ = $1; }
               | listaexpaux ',' exp         { $$ = Ast_prependSibling( $1, $3 ); }
               ;
cmdreturn      : TK_RETURN exp               { $$ = Ast_new( AST_CMD_RETURN , _line );
                                               Ast_addChild( $$, $2 ); }
               | TK_RETURN                   { $$ = Ast_new( AST_CMD_RETURN , _line ); }
               ;
var            : TK_ID                       { $$ = Ast_newFromTokenSv( AST_ID, $1, _line ); }
               | var '[' exp ']'             { $$ = Ast_prependSibling( $1, $3 ); }
               ;
exp      : exp_or                            { $$ = $1; }
         ;
exp_or   : exp_and                           { $$ = $1; }
         | exp_or TK_OR exp_and              { $$ = Ast_new( AST_EXP_OR , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_and  : exp_eq                            { $$ = $1; }
         | exp_and TK_AND exp_eq             { $$ = Ast_new( AST_EXP_AND , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_eq   : exp_rel                           { $$ = $1; }
         | exp_eq '=' exp_rel                { $$ = Ast_new( AST_EXP_EQ , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_eq TK_UNEQUALS exp_rel        { $$ = Ast_new( AST_EXP_UNEQ , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_rel  : exp_add                           { $$ = $1; }
         | exp_rel '<' exp_add               { $$ = Ast_new( AST_EXP_L , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_rel '>' exp_add               { $$ = Ast_new( AST_EXP_G , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_rel TK_LEQUALS exp_add        { $$ = Ast_new( AST_EXP_LEQ , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_rel TK_GEQUALS exp_add        { $$ = Ast_new( AST_EXP_GEQ , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_add  : exp_mult                          { $$ = $1; }
         | exp_add '+' exp_mult              { $$ = Ast_new( AST_EXP_ADD , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_add '-' exp_mult              { $$ = Ast_new( AST_EXP_SUB , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_mult : exp_base                          { $$ = $1; }
         | exp_mult '*' exp_base             { $$ = Ast_new( AST_EXP_MULT , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         | exp_mult '/' exp_base             { $$ = Ast_new( AST_EXP_DIV , _line );
                                               Ast_addChild( $$, $1 );
                                               Ast_addChild( $$, $3 ); }
         ;
exp_base : TK_VALINT                         { $$ = Ast_newFromTokenIv( AST_VALINT, $1, _line ); }
         | TK_VALSTRING                      { $$ = Ast_newFromTokenSv( AST_VALSTRING, $1, _line ); }
         | TK_TRUE                           { $$ = Ast_new( AST_TRUE , _line ); }
         | TK_FALSE                          { $$ = Ast_new( AST_FALSE , _line ); }
         | var                               { $$ = Ast_new( AST_VAR , _line );
                                               Ast_addChildren( $$, $1 ); }
         | TK_NEW '[' exp ']' tipo           { $$ = Ast_new( AST_EXP_NEW , _line );
                                               Ast_addChild( $$, $3 );
                                               Ast_addChild( $$, $5 ); }
         | '(' exp ')'                       { $$ = $2; }
         | chamada                           { $$ = $1; }
         | TK_NOT exp_base                   { $$ = Ast_new( AST_EXP_NOT , _line );
                                               Ast_addChild( $$, $2 ); }
         | '-' exp_base                      { $$ = Ast_new( AST_EXP_NEG , _line );
                                               Ast_addChild( $$, $2 ); }
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
      Ast_print( _program );
      fprintf( stdout, "\n" );
      return 0;
   }
}

