%{
/**
 * @file    lexico.l
 * @author  lhpelosi
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Tipos de token
typedef enum
{
   TK_NL = UCHAR_MAX+1,

   TK_IF,
   TK_ELSE,
   TK_END,
   TK_WHILE,
   TK_LOOP,
   TK_FUN,
   TK_RETURN,
   TK_NEW,
   TK_STRING,
   TK_INT,
   TK_CHAR,
   TK_BOOL,
   TK_TRUE,
   TK_FALSE,
   TK_AND,
   TK_OR,
   TK_NOT,
   TK_GEQUALS,
   TK_LEQUALS,
   TK_UNEQUALS,

   TK_VALSTRING,
   TK_VALINT,
   TK_ID,

   ERROR

} TokenType;

// Union pra guardar a informacao semantica do token
typedef union
{
   int ival;
   char * sval;
} Value;
Value yylval;

// Estrutura que representa um token
typedef struct token
{
   // Seu tipo
   TokenType type;

   // Linha no arquivo original
   unsigned int line;

   // Valor semantico
   Value value;

} Token;

// Salva informacao semantica de inteiro
void setTokenInt( char * s );

// Salva informacao semantica de string
void setTokenString( char * s );

// Processa escapes da string lida e retira aspas
void processString();

// Processa a contagem das linhas
void processNewLine();

// Le o proximo token e o aloca numa struct token
Token * getToken();

// Numero da linha corrente de leitura do arquivo
unsigned int _line = 1;

%}

%option noyywrap

%%
[" "\t]                 ; // Ignora espacos e tabulacoes
"//".*                  ; // Comentario C++
"/*"([^*]|"\n"|"*"[^/])*"*"*"*/"	processNewLine(); // Comentario C
("\n"[ \t]*)+         { processNewLine(); return TK_NL; }

"if"                    return TK_IF;
"else"                  return TK_ELSE;
"end"                   return TK_END;
"while"                 return TK_WHILE;
"loop"                  return TK_LOOP;
"fun"                   return TK_FUN;
"return"                return TK_RETURN;
"new"                   return TK_NEW;
"string"                return TK_STRING;
"int"                   return TK_INT;
"char"                  return TK_CHAR;
"bool"                  return TK_BOOL;
"true"                  return TK_TRUE;
"false"                 return TK_FALSE;
"and"                   return TK_AND;
"or"                    return TK_OR;
"not"                   return TK_NOT;

[-+*=/<>()\[\]:,]       return yytext[0]; // Tokens de um unico char tem seu proprio valor
">="                    return TK_GEQUALS;
"<="                    return TK_LEQUALS;
"<>"                    return TK_UNEQUALS;

\"(\\.|[^\\"])*\"       { processString(); return TK_VALSTRING; } // String
"0"[xX][0-9A-Fa-f]+     | // Int Hexa
0|[1-9][0-9]*           { setTokenInt( yytext ); return TK_VALINT; } // Int Decimal
[A-Za-z_][A-Za-z0-9_]*  { setTokenString( yytext ); return TK_ID; } // Identificadores

.                       return ERROR;

%%

void setTokenInt( char * s )
{
   yylval.ival = strtol( s, NULL, 0 );
}

void setTokenString( char * s )
{
   yylval.sval = (char*) malloc( (strlen(s)+1) * sizeof(char) );
   strcpy( yylval.sval, s );
}

void processString()
{
   int i, j;

   j = 0;
   for ( i = 1 ; // Comeca depois da aspas
         i < yyleng-1 ; // Le ate antes do ultimo char, que eh aspas
         i++, j++ )
   {
      if ( yytext[i] == '\\' )
      {
         i++; // Avanca o char lido
         switch ( yytext[i] )
         {
            case 'n': yytext[j] = '\n'; break;
            case 't': yytext[j] = '\t'; break;
            default: yytext[j] = yytext[i];
         }
      }
      else yytext[j] = yytext[i];
   }
   yytext[j] = '\0';

   setTokenString( yytext );
}

void processNewLine()
{
   int i;
   for ( i = 0 ; i < yyleng ; i++ )
   {
      if ( yytext[i] == '\n' )
         _line++;
   }
}

Token * getToken()
{
   unsigned int tokenType = yylex();
   if ( !tokenType ) return NULL;

   Token * token = (Token *) malloc( sizeof(Token) );
   token->type = tokenType;
   token->line = _line;
   token->value = yylval;

   return token;
}

int main( int argc, char * argv[] )
{
   Token * token;

   if ( argc > 1 )
      yyin = fopen( argv[1] ,"r" );

   // Enquanto houver tokens, ja os salva na variavel token
   while ( token = getToken() )
   {
      unsigned int tokenType = token->type;
      char * separator = " ";

      if ( tokenType != TK_NL )
         printf( "%d_", token->line );

      // Se for token de apenas um char
      if ( tokenType <= UCHAR_MAX )
      {
         printf( "TOKEN_%c", (unsigned char)tokenType );
      }
      else switch ( tokenType )
      {
         case TK_NL: printf( "NL" ); separator = "\n"; break;

         case TK_IF: printf( "TK_IF" ); break;
         case TK_ELSE: printf( "TK_ELSE" ); break;
         case TK_END: printf( "TK_END" ); break;
         case TK_WHILE: printf( "TK_WHILE" ); break;
         case TK_LOOP: printf( "TK_LOOP" ); break;
         case TK_FUN: printf( "TK_FUN" ); break;
         case TK_RETURN: printf( "TK_RETURN" ); break;
         case TK_NEW: printf( "TK_NEW" ); break;
         case TK_STRING: printf( "TK_STRING" ); break;
         case TK_INT: printf( "TK_INT" ); break;
         case TK_CHAR: printf( "TK_CHAR" ); break;
         case TK_BOOL: printf( "TK_BOOL" ); break;
         case TK_TRUE: printf( "TK_TRUE" ); break;
         case TK_FALSE: printf( "TK_FALSE" ); break;
         case TK_AND: printf( "TK_AND" ); break;
         case TK_OR: printf( "TK_OR" ); break;
         case TK_NOT: printf( "TK_NOT" ); break;
         case TK_GEQUALS: printf( "TK_GEQUALS" ); break;
         case TK_LEQUALS: printf( "TK_LEQUALS" ); break;
         case TK_UNEQUALS: printf( "TK_UNEQUALS" ); break;

         case TK_VALSTRING: printf( "TK_VALSTRING_\"%s\"", token->value.sval ); break;
         case TK_VALINT: printf( "TK_VALINT_\"%d\"", token->value.ival ); break;
         case TK_ID: printf( "TK_ID_\"%s\"", token->value.sval ); break;

         case ERROR: printf( "InvalidToken" ); break;

         default: printf( "\n>>> Estado invalido!\n" );
      }
      
      printf( separator );
   }

   return 0;
}

