%locations

%{
    #define YYDEBUG 1
    #include <stdio.h>
    #include "lex.yy.c"
    extern int yylex();
    struct SyntaxTreeNode *root;
    %}

%union{
    struct SyntaxTreeNode* node;
}

%token <node> TYPE STRUCT RETURN IF ELSE WHILE
%token <node> INT 
%token <node> FLOAT 
%token <node> ID
%token <node> COMMA ASSIGNOP SEMI
%token <node> RELOP
%token <node> PLUS MINUS MUL DIV
%token <node> AND OR DOT NOT
%token <node> LP RP LB RB LC RC

%type <node> Program ExtDefList ExtDef Specifier ExtDecList StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def
%type <node> DecList Dec Exp Args

%right ASSIGNOP
%left AND OR
%left RELOP
%left PLUS MINUS
%left MUL DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%start Program

%%
Program: ExtDefList {$$=createNode("Program", @1.first_line, NULL); addChildNode($$, $1); root=$$;};

ExtDefList: /* empty */ {
                $$=createNode("ExtDefList", yylineno, NULL);
                }
                | ExtDef ExtDefList {
                    $$=createNode("ExtDefList", @1.first_line, NULL);
                    addChildNode($$, $1);
                    addChildNode($$, $2);
                }; 

ExtDef: Specifier ExtDecList SEMI{
                    $$=createNode("ExtDef", @1.first_line, NULL);
                    addChildNode($$, $1);
                    addChildNode($$, $2);
                    addChildNode($$, $3);
                }
            | Specifier ExtDecList error SEMI {yyerrok;}
            | Specifier SEMI {
                $$=createNode("ExtDef", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
            }
            | Specifier FunDec CompSt{
                $$=createNode("ExtDef", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
            }
            | Specifier FunDec error SEMI {yyerrok;}
            ;

ExtDecList: VarDec{
                        $$=createNode("ExtDecList", @1.first_line, NULL);
                        addChildNode($$, $1);
                }
                | VarDec COMMA ExtDecList{
                    $$=createNode("ExtDecList", @1.first_line, NULL);
                    addChildNode($$, $1);
                    addChildNode($$, $2);
                    addChildNode($$, $3);
                };

Specifier: TYPE {
                    $$=createNode("Specifier", @1.first_line, $1->lexeme);
                    addChildNode($$, $1);
                }   
                | StructSpecifier{
                    $$=createNode("Specifier", @1.first_line, NULL);
                    addChildNode($$, $1);
                };

StructSpecifier: STRUCT OptTag LC DefList RC{
                            $$=createNode("StructSpecifier", @1.first_line, NULL);
                            addChildNode($$, $1);
                            addChildNode($$, $2);
                            addChildNode($$, $3);
                            addChildNode($$, $4);
                            addChildNode($$, $5); 
                        }
                        | STRUCT Tag{
                            $$=createNode("StructSpecifier", @1.first_line, NULL);
                            addChildNode($$, $1);
                            addChildNode($$, $2);
                        };

OptTag: /* empty */ {
                $$=createNode("OptTag", yylineno, NULL);
            }
            | ID{
                $$=createNode("OptTag", @1.first_line, $1->lexeme);
                addChildNode($$, $1);
            };

Tag: ID {
            $$=createNode("Tag", @1.first_line, $1->lexeme);
            addChildNode($$, $1);
        };

VarDec: ID{
                $$=createNode("VarDec", @1.first_line, $1->lexeme);
                addChildNode($$, $1);
            }
            | VarDec LB INT RB{
                $$=createNode("VarDec", @1.first_line, $1->lexeme);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
                addChildNode($$, $4);
            } 
            | VarDec LB error RB {yyerrok;};

FunDec: ID LP VarList RP{
                $$=createNode("FunDec", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
                addChildNode($$, $4);
            }
            | ID LP RP{
                $$=createNode("FunDec", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
            }
            | ID LP error RP {yyerrok;}; 

VarList: ParamDec COMMA VarList{
                $$=createNode("VarList", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
            }
            | ParamDec {
                $$=createNode("VarList", @1.first_line, NULL);
                addChildNode($$, $1);
            };

ParamDec: Specifier VarDec{
                    $$=createNode("ParamDec", @1.first_line, NULL);
                    addChildNode($$, $1);
                    addChildNode($$, $2);
                }
                | Specifier error {yyerrok;}; 

CompSt: LC DefList StmtList RC {
                $$=createNode("CompSt", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
                addChildNode($$, $4);
            };

StmtList: /* empty */{
                $$=createNode("StmtList", yylineno, NULL);
            }
            | Stmt StmtList {
                $$=createNode("StmtList", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
            }
            | Stmt error SEMI StmtList {yyerrok;};

Stmt: Exp SEMI {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
        }
        | CompSt {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
        }
        | RETURN Exp SEMI {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | RETURN Exp error {yyerrok;}
        | RETURN error SEMI {yyerrok;}
        | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
            addChildNode($$, $4);
            addChildNode($$, $5);
        }
        | IF LP error RP Stmt %prec LOWER_THAN_ELSE {yyerrok;}
        | IF LP Exp RP Stmt ELSE Stmt {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
            addChildNode($$, $4);
            addChildNode($$, $5);
            addChildNode($$, $6);
            addChildNode($$, $7);
        }
        | IF LP Exp RP error ELSE Stmt
        | IF LP error RP Stmt ELSE Stmt {yyerrok;}
        | WHILE LP Exp RP Stmt {
            $$=createNode("Stmt", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
            addChildNode($$, $4);
            addChildNode($$, $5);
        }
        | WHILE LP error RP Stmt
        | WHILE LP Exp RP error SEMI {yyerrok;}
        | Exp error SEMI {yyerrok;};

DefList: /* empty */ {
                $$=createNode("DefList", yylineno, NULL);
            }
            | Def DefList {
                $$=createNode("DefList", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
            };

Def: Specifier DecList SEMI {
            $$=createNode("Def", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | error SEMI {yyerrok;};

DecList: Dec {
                $$=createNode("DecList", @1.first_line, NULL);
                addChildNode($$, $1);
            }
            | Dec COMMA DecList {
                $$=createNode("DecList", @1.first_line, NULL);
                addChildNode($$, $1);
                addChildNode($$, $2);
                addChildNode($$, $3);
            };

Dec: VarDec {
            $$=createNode("Dec", @1.first_line, NULL);
            addChildNode($$, $1);
        }
        | VarDec ASSIGNOP Exp {
            $$=createNode("Dec", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        };

Exp: Exp ASSIGNOP Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp AND Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp OR Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp RELOP Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp PLUS Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp MINUS Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp MUL Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp DIV Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | LP Exp RP {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | MINUS Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
        }
        | NOT Exp {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
        }
        | ID LP Args RP {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
            addChildNode($$, $4);
        }
        | ID LP RP {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp LB Exp RB {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
            addChildNode($$, $4); 
        }
        | Exp LB error RB {yyerrok;}
        | Exp DOT ID {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | ID {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
        }
        | INT {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
        }
        | FLOAT {
            $$=createNode("Exp", @1.first_line, NULL);
            addChildNode($$, $1);
        };

Args: Exp COMMA Args {
            $$=createNode("Args", @1.first_line, NULL);
            addChildNode($$, $1);
            addChildNode($$, $2);
            addChildNode($$, $3);
        }
        | Exp {
            $$=createNode("Args", @1.first_line, NULL);
            addChildNode($$, $1);
        };

%%

yyerror(char* msg) {
    printf("Error type B at Line %d: near %s\n", yylineno, yytext);
}