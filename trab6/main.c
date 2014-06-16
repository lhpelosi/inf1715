
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "asm.h"

extern FILE* yyin;
extern int yyparse();
extern int yydebug;

extern IR* ir;

int main(int argc, char** argv) {
	int err;
	FILE* outputFile;
	char outputFileName[256];

	if (argc < 2) {
		fprintf(stderr, "Uso: %s arquivo.m0.ir\n", argv[0]);
		exit(1);
	}
	yyin = fopen(argv[1], "r");
	err = yyparse();
	fclose(yyin);
	if (err != 0) {
		fprintf(stderr, "Error reading input file.\n");
		exit(1);
	}

   strcpy( outputFileName, argv[1] );
   strcpy( &(outputFileName[ strlen(argv[1])-6 ]), ".s" );
   outputFile = fopen( outputFileName, "w" );

	//IR_dump( ir, stdout );
	Asm_write( ir, outputFile );
	
   fclose( outputFile );
	return 0;
}

