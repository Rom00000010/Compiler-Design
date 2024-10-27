%option yylineno
%{
    #include "../include/syntax.tab.h"
    #include "../include/syntax_tree.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

delim [ \t]
ws {delim}+
digit [0-9]
nz_digit [1-9]
INT ({nz_digit}{digit}*)|0
FLOAT {INT}\.{digit}+
letter [_a-zA-Z]
character [_a-zA-Z0-9]
ID {letter}{character}*

%%
{ws} {}
"\n" {}
"int" { yylval.node = createNode("TYPE", yylineno, "int"); return TYPE; }
"float" { yylval.node = createNode("TYPE", yylineno, "float"); return TYPE; }
"struct" { yylval.node = createNode("STRUCT", yylineno, "struct"); return STRUCT; }
"return" { yylval.node = createNode("RETURN", yylineno, "return"); ;return RETURN; }
"if" { yylval.node = createNode("IF", yylineno, "if"); return IF; }
"else" { yylval.node = createNode("ELSE", yylineno, "else"); return ELSE; }
"while" { yylval.node = createNode("WHILE", yylineno, "while"); return WHILE; }
{INT} { yylval.node = createNode("INT", yylineno, yytext); return INT; }
{FLOAT} { yylval.node = createNode("FLOAT", yylineno, yytext); return FLOAT; }
{ID} { yylval.node = createNode("ID", yylineno, yytext); return ID; }
";" { yylval.node = createNode("SEMI", yylineno, ";"); return SEMI; }
"," { yylval.node = createNode("COMMA", yylineno, ","); return COMMA; }
"=" { yylval.node = createNode("ASSIGNOP", yylineno, "="); return ASSIGNOP; }
>|<|>=|<=|==|!= { yylval.node = createNode("RELOP", yylineno, "relop"); return RELOP; }
"+" { yylval.node = createNode("PLUS", yylineno, "+"); return PLUS; }
"-" { yylval.node = createNode("MINUS", yylineno, "-"); return MINUS; }
"*" { yylval.node = createNode("MUL", yylineno, "*"); return MUL; }
"/" { yylval.node = createNode("DIV", yylineno, "/"); return DIV; }
"&&" { yylval.node = createNode("AND", yylineno, "&&"); return AND; }
"||" { yylval.node = createNode("OR", yylineno, "||"); return OR; }
"." { yylval.node = createNode("DOT", yylineno, "."); return DOT; }
"!" { yylval.node = createNode("NOT", yylineno, "!"); return NOT; }
"(" { yylval.node = createNode("LP", yylineno, "("); return LP; }
")" { yylval.node = createNode("RP", yylineno, ")"); return RP; }
"[" { yylval.node = createNode("LB", yylineno, "["); return LB; }
"]" { yylval.node = createNode("RB", yylineno, "]"); return RB; }
"{" { yylval.node = createNode("LC", yylineno, "{"); return LC; }
"}" { yylval.node = createNode("RC", yylineno, "}"); return RC; }
{digit}+ {lexerr(); return INT;} // badly formatted int
{INT}*\.{digit}* {lexerr(); return FLOAT;} // badly formatted float
[0-9.]+ {lexerr(); return FLOAT;} // badly formatted float
({digit}+{ID}) {lexerr(); return ID;} // ID start with digit
"~" {lexerr();} // unsupported calculate symbol
"&" {lexerr(); return MINUS;} // unsupported calculate symbol
"@" {lexerr(); return MINUS;} // unsupported calculate symbol
. {lexerr();printf("1");}
%%

inline void lexerr()
{
    printf("Error type A at Line %d: Mysterious characters '%s'\n", yylineno, yytext);
}
