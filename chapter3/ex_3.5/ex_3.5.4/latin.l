%option noyywrap case-insensitive

%{
	#include <ctype.h>
	char *global_str = NULL;
%}

%%
[bcdfghjklmnpqrstvwxyz][a-z]*		{
				char temp = 0;
				global_str = malloc(strlen(yytext) + 2 + 1);	
				if (!global_str) {
					perror("fail to malloc");
					exit(-1);
				}
				memset(global_str, 0, strlen(yytext) + 2 + 1);
				temp = *(yytext);
				if (isupper(temp)) {
					*(yytext + 1) = toupper(*(yytext + 1));
					temp = tolower(temp);
				}

				strcpy(global_str, yytext + 1);
				sprintf(global_str + strlen(global_str), "%c%s", temp, "ay");
				fwrite(global_str, strlen(global_str), 1, yyout);
				free(global_str);
				}

[aeio][a-z]*			{	
				global_str = malloc(strlen(yytext) + 2 + 1);	
				if (!global_str) {
					perror("fail to malloc");
					exit(-1);
				}
				memset(global_str, 0, strlen(yytext) + 2 + 1);
				strcpy(global_str, yytext);
				strcat(global_str, "ay");
				fwrite(global_str, strlen(global_str), 1, yyout);
				free(global_str);
				}
[^a-z]+				{
				ECHO;
				}

<<EOF>> 			{
				printf("end\n");
				return 0;
				}
%%

int main(int argc, char *argv[])
{ 
	FILE *fp_in = NULL;
	FILE *fp_out = NULL;

	if (argc != 3) {
	fprintf(stderr, "latin inputfile outputfile\n");
	return -1;
	}

	fp_in = fopen(argv[1], "r");
	if (!fp_in) {
	fprintf(stderr, "fail to open file %s\n", argv[1]);
	goto end;
	}

	fp_out = fopen(argv[2], "w+");
	if (!fp_out) {
	fprintf(stderr, "fail to open file %s\n", argv[2]);
	}

	yyin = fp_in;
	yyout = fp_out;
	yyrestart(fp_in);
	yylex();

end:
	if (fp_in)
		fclose(fp_in);

	if (fp_out)
		fclose(fp_out);
}
