%option noyywrap
%x IFILE FILENAME


%{
	struct bufstack {
		struct bufstack *prev;
		YY_BUFFER_STATE bs;
		int lineno;
		char *filename;
		FILE *f;

	};

	struct bufstack *curbs = NULL;
	char *curfilename = NULL;
	int newfile(char *fn);
	int popfile(void);
%}

%%
^#[ \t]*include[ \t]*[<\"]	{ BEGIN IFILE; }
<IFILE>[^ \t\n\"]+		{
				int c;
				while((c = input()) && c != '\n');
				
				yylineno ++;
				if (!newfile(yytext))
					yyterminate();
				BEGIN INITIAL;
				}

<IFILE>.|\n			{
				fprintf(stderr, "%4d bad include line\n", yylineno);
				yyterminate();
				}

<<EOF>>				{
				if (!popfile()) yyterminate();	
				}

^.				{ fprintf(yyout, "%4d %s %4d\n", yylineno, yytext, yyleng);}
^\n				{ fprintf(yyout, "%4d %s %4d\n", yylineno ++, yytext, yyleng); }
\n				{ ECHO; yylineno ++; }
.				{ ECHO; }
%%

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "need filename.\n");
		return 1;
	}

	if (newfile(argv[1]))
		yylex();

	return 0;
}

int newfile(char *fn)
{
	FILE *f = NULL;
	struct bufstack *bs = NULL;


	f = fopen(fn, "r"); 
	if (!f) {
		perror(fn);
		return 0;
	}

	bs = malloc(sizeof(struct bufstack));
	if (!bs) {
		perror("fail to malloc");
		return 0;
	}

	if (curbs) 
		curbs->lineno = yylineno;

	bs->prev = curbs;

	bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
	bs->f = f;
	bs->filename = fn;
	yy_switch_to_buffer(bs->bs);
	curbs = bs;
	yylineno = 1;
	curfilename = fn;
	return 1;

}

int popfile(void)
{
	struct bufstack *bs = curbs;
	struct bufstack *prevbs = NULL;

	if (!bs)
		return 0;

	prevbs = curbs->prev;
	yy_delete_buffer(bs->bs);
	fclose(bs->f);

	free(bs);

	if (!prevbs)
		return 0;

	curfilename = prevbs->filename;
	yylineno = prevbs->lineno;	
	yy_switch_to_buffer(prevbs->bs);
	curbs = prevbs;

	return 1;
}
