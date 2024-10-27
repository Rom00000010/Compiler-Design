#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/syntax_tree.h"
extern int yylex(void);
extern int yyparse();
extern SyntaxTreeNode *root;
extern FILE *yyin;
extern int yydebug;

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		if (!(yyin = fopen(argv[1], "r")))
		{
			perror(argv[1]);
			return 1;
		}
	}
	srand(time(0));
	if (!yyparse())
	{
		dfs(root);
		freeSyntaxTree(root);
	}
	return 0;
}