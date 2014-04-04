%{
/**
 * @file    yacc.y
 * @author  lhpelosi
 */

# include <stdio.h>
extern FILE * yyin;
int yydebug=1;

%}

%union
{
   int ival;
   char * sval;
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
programa       : nl programaaux
               | programaaux
               ;
programaaux    : decl
               | programaaux decl
               ;
decl           : funcao
               | global
               ;
nl             : TK_NL
               | nl TK_NL
               ;
global         : declvar nl
               ;
funcao         : TK_FUN TK_ID '(' params ')' funtipo nl bloco TK_END nl
               ;
funtipo        : //vazio
               | ':' tipo
               ;
bloco          : //vazio
               | listadeclvar
               | listacomando
               | listadeclvar listacomando
               ;
listadeclvar   : declvar nl
               | listadeclvar declvar nl
               ;
listacomando   : comando nl
               | listacomando comando nl
               ;
params         : //vazio
               | paramsaux
               ;
paramsaux      : parametro
               | paramsaux ',' parametro
               ;
parametro      : TK_ID ':' tipo
               ;
tipo           : tipobase
               | '[' ']' tipo
               ;
tipobase       : TK_INT
               | TK_BOOL
               | TK_CHAR
               | TK_STRING
               ;
declvar        : TK_ID ':' tipo
               ;
comando        : cmdif
               | cmdwhile
               | cmdatrib
               | cmdreturn
               | chamada
               ;
cmdif          : TK_IF exp nl bloco cmdelseif cmdelse TK_END
               ;
cmdelseif      : //vazio
               | cmdelseif TK_ELSE TK_IF exp nl bloco
               ;
cmdelse        : //vazio
               | TK_ELSE nl bloco
               ;
cmdwhile       : TK_WHILE exp nl bloco TK_LOOP
               ;
cmdatrib       : var '=' exp
               ;
chamada        : TK_ID '(' listaexp ')'
               ;
listaexp       : //vazio
               | listaexpaux
               ;
listaexpaux    : exp
               | listaexpaux ',' exp
               ;
cmdreturn      : TK_RETURN exp
               | TK_RETURN
               ;
var            : TK_ID
               | var '[' exp ']'
               ;
exp      : exp_or
         ;
exp_or   : exp_and
         | exp_or TK_OR exp_and
         ;
exp_and  : exp_eq
         | exp_and TK_AND exp_eq
         ;
exp_eq   : exp_rel
         | exp_eq '=' exp_rel
         | exp_eq TK_UNEQUALS exp_rel
         ;
exp_rel  : exp_add
         | exp_rel '<' exp_add
         | exp_rel '>' exp_add
         | exp_rel TK_LEQUALS exp_add
         | exp_rel TK_GEQUALS exp_add
         ;
exp_add  : exp_mult
         | exp_add '+' exp_mult
         | exp_add '-' exp_mult
         ;
exp_mult : exp_base
         | exp_mult '*' exp_base
         | exp_mult '/' exp_base
         ;
exp_base : TK_VALINT | TK_VALSTRING
         | TK_TRUE | TK_FALSE
         | var
         | TK_NEW '[' exp ']' tipo
         | '(' exp ')'
         | chamada
         | TK_NOT exp_base
         | '-' exp_base
         ;
%%

yyerror( char *s )
{
   fprintf( stderr, "%s\n", s );
}

int main( int argc, char * argv[] )
{
   if ( argc > 1 )
      yyin = fopen( argv[1] ,"r" );

   return( yyparse() );
}

