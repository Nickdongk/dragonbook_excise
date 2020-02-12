%option noyywrap

%{
	

%}

%%
[ \t]+		{
		fprintf(yyout,"%s", " ");
		}
[^ \t]*		{
		ECHO;
		}
%%

int main(int argc, char *argv[])
{
	FILE *fp_in = NULL, *fp_out = NULL;

	if (argc != 3) {
		fprintf(stderr, "rm_delim inputfile outputfile\n");
		return -1;
	}

	fp_in = fopen(argv[1], "r");
	if (!fp_in) {
		fprintf(stderr, "fail open %s\n", argv[1]);
		return -1;
	}

	fp_out = fopen(argv[2], "w+");
	if (!fp_out) {
		fprintf(stderr, "fail open %s\n", argv[2]);
		goto end;
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
