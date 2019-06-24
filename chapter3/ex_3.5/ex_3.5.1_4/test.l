%option noyywrap
%x STRING

%{
 char *str_result = NULL;
 
%}

%%
\n	{	}
\"	{ BEGIN STRING; }
<STRING>[^\"\\]*	{
			if (str_result != NULL) {
				char *str_tmp = str_result;
				str_result = malloc(strlen(str_tmp) + yyleng + 1);
				if (!str_result) {
					printf("malloc str fail\n");
					exit(1);
				}
				memset(str_result, 0, strlen(str_tmp) + yyleng + 1);
				strcpy(str_result, str_tmp);
				strcat(str_result, yytext);
				if (str_tmp)
					free(str_tmp);
			} else {
				str_result = strdup(yytext);
			}
				printf("%s |\n", str_result);
			}
<STRING>(\\\")|(\\\\)		{
			char *str_tmp = str_result;
			str_result = malloc(strlen(str_tmp) +  yyleng + 1);
			if (!str_result) {
				printf("malloc str fail\n");
				exit(1);
			}
			memset(str_result, 0, strlen(str_tmp) + yyleng + 1);
			strcpy(str_result, str_tmp);

			if (*(yytext + 1) == '\"')
				strcat(str_result, "\"");
			else if (*(yytext + 1) == '\\')
				strcat(str_result, "\\");
			if (str_tmp)
				free(str_tmp);
			printf("%s |\n", str_result);
			}

<STRING>\"		{
			printf("%s |\n", str_result);
			BEGIN INITIAL;
			}

<<EOF>>			{
				return 0;
			}
%%

int main(int argc, char *argv[])
{
	FILE *fp = NULL;

	if (argc != 2)
		fprintf(stderr, "string_test filename\n");
	
	fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "fail to open %s\n", argv[1]);
		return -1;
	}

	yyin = fp;
	yyrestart(fp);
	yylex();

	fclose(fp);
	return 0;

}
