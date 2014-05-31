%{
/**
 * @file    lexico.l
 * @author  lhpelosi
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "y.tab.h"

// Salva informacao semantica de inteiro
void setTokenInt( char * s );

// Salva informacao semantica de string
void setTokenString( char * s );

// Salva informacao do numero da linha, no caso de tokens simbolicos
void setLine();

// Processa escapes da string lida e retira aspas
void processString();

// Processa a contagem das linhas
void processNewLine();

// Notifica erros de leitura do lexico
void lexicalError();

// Numero da linha corrente de leitura do arquivo
unsigned int _line = 1;

// Identifica erros detectados no lexico ( 0 == sem erros )
extern unsigned int _lexicalError;

%}

%option noyywrap

%%
[" "\t]                 ; // Ignora espacos e tabulacoes
"//".*                  ; // Comentario C++
"/*"([^*]|"\n"|"*"[^/])*"*"*"*/"	processNewLine(); // Comentario C
("\n"[ \t]*)+         { processNewLine(); return TK_NL; }

"if"                    { setLine(); return TK_IF; }
"else"                  { setLine(); return TK_ELSE; }
"end"                   { setLine(); return TK_END; }
"while"                 { setLine(); return TK_WHILE; }
"loop"                  { setLine(); return TK_LOOP; }
"fun"                   { setLine(); return TK_FUN; }
"return"                { setLine(); return TK_RETURN; }
"new"                   { setLine(); return TK_NEW; }
"string"                { setLine(); return TK_STRING; }
"int"                   { setLine(); return TK_INT; }
"char"                  { setLine(); return TK_CHAR; }
"bool"                  { setLine(); return TK_BOOL; }
"true"                  { setLine(); return TK_TRUE; }
"false"                 { setLine(); return TK_FALSE; }
"and"                   { setLine(); return TK_AND; }
"or"                    { setLine(); return TK_OR; }
"not"                   { setLine(); return TK_NOT; }

[-+*=/<>()\[\]:,]       { setLine(); return yytext[0]; } // Tokens de um unico char tem seu proprio valor
">="                    { setLine(); return TK_GEQUALS; }
"<="                    { setLine(); return TK_LEQUALS; }
"<>"                    { setLine(); return TK_UNEQUALS; }

\"(\\.|[^\\"])*\"       { processString(); return TK_VALSTRING; } // String
"0"[xX][0-9A-Fa-f]+     | // Int Hexa
0|[1-9][0-9]*           { setTokenInt( yytext ); return TK_VALINT; } // Int Decimal
[A-Za-z_][A-Za-z0-9_]*  { setTokenString( yytext ); return TK_ID; } // Identificadores

.                       lexicalError();

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

void setLine()
{
   yylval.line = _line;
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

void lexicalError()
{
   _lexicalError++;
   fprintf( stderr, "Linha %d: Token invalido \"%s\"\n", _line, yytext );
}

