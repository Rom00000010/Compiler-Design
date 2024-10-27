#include "time.h"
#include "../include/syntax_tree.h"
#include "../include/stack.h"

ScopeStack *stack;
Scope *currentScope;

void error(int type, int line, char *errorMessage, char *obj)
{
    printf("Error type %d at Line %d: %s \"%s\"\n", type, line, errorMessage, obj);
}

// 函数生成一个随机的名称，长度为 len
char *generate_random_name(int len)
{
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *random_name = (char *)malloc((len + 1) * sizeof(char)); // 分配内存
    if (random_name)
    {
        for (int i = 0; i < len; ++i)
        {
            int key = rand() % (int)(sizeof(charset) - 1);
            random_name[i] = charset[key];
        }
        random_name[len] = '\0'; // 以 '\0' 结尾
    }
    return random_name;
}

SyntaxTreeNode *createNode(const char *label, int line, const char *lexeme)
{
    SyntaxTreeNode *node = (SyntaxTreeNode *)malloc(sizeof(SyntaxTreeNode));
    node->label = my_strdup(label);
    node->line = line;
    node->lexeme = lexeme ? my_strdup(lexeme) : NULL;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChildNode(SyntaxTreeNode *parent, SyntaxTreeNode *child)
{
    if (!parent || !child)
        return;
    parent->children = (SyntaxTreeNode **)realloc(parent->children, sizeof(SyntaxTreeNode *) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

void printTree(SyntaxTreeNode *node, int depth)
{
    if (!node)
        return;
    for (int i = 0; i < depth; i++)
        printf("  ");

    if (node->lexeme)
    {
        printf("%s: %s (%d)\n", node->label, node->lexeme, node->line);
    }
    else
    {
        printf("%s (%d)\n", node->label, node->line);
    }

    for (int i = 0; i < node->childCount; i++)
    {
        printTree(node->children[i], depth + 1);
    }
}

void freeSyntaxTree(SyntaxTreeNode *node)
{
    if (!node)
        return;
    free(node->label);
    if (node->lexeme)
        free(node->lexeme);
    for (int i = 0; i < node->childCount; i++)
    {
        freeSyntaxTree(node->children[i]);
    }
    free(node->children);
    free(node);
}

// 语义分析
varSymbol *expHandler(SyntaxTreeNode *node);
void varDecHandler(SyntaxTreeNode *varDecNode, Type currentType, int mode, Type structType)
{
    // basic dimension
    SyntaxTreeNode *idNode = varDecNode->children[0];
    if (strcmp(idNode->label, "ID") == 0)
    {
        char *name = idNode->lexeme;
        Symbol *varSymbol = createVarSymbol(name, currentType);

        // 错误类型3:  变量出现重复定义，或变量与前面定义过的结构体名字重复
        Symbol *symbol = findSymbol(((structScope *)(currentScope))->table, name);
        if (symbol != NULL && !mode)
            error(3, varDecNode->line, "Redifined variable", name);
        else if (symbol != NULL && mode)
            error(15, varDecNode->line, "Redefined field", name);

        if (mode == 0)
            define(currentScope, name, varSymbol);
        else if (mode == 1)
        {
            addField(structType, name, currentType, idNode->line);
        }
    }

    //  recursive dimension
    else
    {
        currentType = createArrayType(currentType, atoi(varDecNode->children[2]->lexeme));
        varDecHandler(idNode, currentType, mode, structType);
    }
}

void decListHandler(SyntaxTreeNode *decListNode, Type currentType, int mode, Type structType)
{
    // current var
    SyntaxTreeNode *varDecNode = decListNode->children[0];

    //  dec instead of vardec
    if (strcmp(varDecNode->label, "Dec") == 0)
    {
        varDecHandler(varDecNode->children[0], currentType, mode, structType);
        // 错误类型15: 在定义时对域进行初始化（例如struct A { int a = 0; }）
        if (varDecNode->childCount == 3 && mode)
        {
            error(15, varDecNode->line, "Initializing field when define struct", varDecNode->children[0]->children[0]->lexeme);
        }
        else if (varDecNode->childCount == 3 && !mode)
        {   
            // local variable declaration and assignment
            varSymbol *sym = expHandler(varDecNode->children[2]);
            if (sym!=NULL && !equal(sym->type, currentType))
            {
                error(5, varDecNode->children[2]->line, "Type mismatch", "");
            }
        }
    }
    else
        varDecHandler(varDecNode, currentType, mode, structType);

next:
    // recursive
    if (decListNode->childCount > 1)
    {
        decListHandler(decListNode->children[2], currentType, mode, structType);
    }
}

Type structDefHandler(SyntaxTreeNode *node, Type recurStructType);
void varDefHandler(SyntaxTreeNode *node, Type structType, int mode)
{
    SyntaxTreeNode *typeNode = node->children[0]->children[0];
    SyntaxTreeNode *decListNode = node->children[1];
    Type currentType;

    // basic type variables
    if (strcmp(typeNode->label, "TYPE") == 0)
        currentType = createBasicType((strcmp(typeNode->lexeme, "int") == 0) ? 1 : 2);

    // Struct type variables
    else if (strcmp(typeNode->label, "StructSpecifier") == 0 && strcmp(typeNode->children[1]->label, "Tag") == 0)
    {
        Symbol *symbol = resolve(stack, currentScope, typeNode->children[1]->lexeme);

        // 错误类型17：直接使用未定义过的结构体来定义变量。
        if (symbol == NULL)
        {
            error(17, typeNode->children[1]->line, "Undefined structure", typeNode->children[1]->lexeme);
            return;
        }

        currentType = ((structSymbol *)symbol)->type;
    }

    // Define type first, then define variables
    else if (strcmp(typeNode->label, "StructSpecifier") == 0 && strcmp(typeNode->children[1]->label, "OptTag") == 0)
        currentType = structDefHandler(typeNode, structType);

    decListHandler(decListNode, currentType, mode, structType);
}

// ---------------------------------------------------------------- ⬆ Global Variables Definition

void paramVarDecHandler(SyntaxTreeNode *node, Type currentType, char **param, Type *paramType, int paramCnt)
{
    // Basic dimension
    SyntaxTreeNode *idNode = node->children[0];
    if (strcmp(idNode->label, "ID") == 0)
    {
        char *name = idNode->lexeme;
        Symbol *varSymbol = createVarSymbol(name, currentType);

        param[paramCnt] = my_strdup(name);
        paramType[paramCnt] = currentType;
    }
    //  Recursive dimension
    else
    {
        currentType = createArrayType(currentType, atoi(node->children[2]->lexeme));
        paramVarDecHandler(idNode, currentType, param, paramType, paramCnt);
    }
}

void paramDecHandler(SyntaxTreeNode *node, char **param, Type *paramType, int paramCnt)
{
    // Basic type
    Type currentType;
    if (strcmp(node->children[0]->children[0]->label, "TYPE") == 0)
    {
        currentType = createBasicType((strcmp(node->children[0]->children[0]->lexeme, "int") == 0) ? 1 : 2);
    }
    // Struct type
    else
    {
        SyntaxTreeNode *tagNode = node->children[0]->children[0]->children[1];
        if (strcmp(tagNode->label, "OptTag") == 0)
            return;

        Symbol *sym = resolve(stack, currentScope, tagNode->children[0]->lexeme);
        if (sym == NULL)
        {
            return;
        }
        currentType = ((structSymbol *)sym)->type;
    }
    paramVarDecHandler(node->children[1], currentType, param, paramType, paramCnt);
}

void varListHandler(SyntaxTreeNode *node, char **param, Type *paramType, int paramCnt)
{
    paramDecHandler(node->children[0], param, paramType, paramCnt);

    if (node->childCount > 1)
        varListHandler(node->children[2], param, paramType, paramCnt + 1);
}

int paramCount(SyntaxTreeNode *node, int init)
{
    if (node->childCount == 1)
        return init + 1;
    return paramCount(node->children[2], init + 1);
}

void defListHandler(SyntaxTreeNode *node, Type structType);
void funcDefHandler(SyntaxTreeNode *node)
{
    SyntaxTreeNode *typeNode = node->children[0]->children[0];
    SyntaxTreeNode *funcDecNode = node->children[1];

    // Name
    char *name = funcDecNode->children[0]->lexeme;

    Type retType;
    // Return type
    if (strcmp(typeNode->label, "TYPE") == 0)
    {
        retType = createBasicType((strcmp(typeNode->lexeme, "int") == 0) ? 1 : 2);
    }
    else if (strcmp(typeNode->children[1]->label,"Tag") == 0)
    {
        char *structName = typeNode->children[1]->children[0]->lexeme;
        Symbol *sym = resolve(stack, currentScope, structName);
        retType = ((structSymbol *)sym)->type;
    }

    // Parameter
    int paramCnt = 0;
    char **param;
    Type *paramType;
    if (funcDecNode->childCount == 3)
        paramCnt = 0;
    else
    {
        paramCnt = paramCount(funcDecNode->children[2], 0);
        param = (char **)malloc(sizeof(char *) * paramCnt);
        paramType = (Type *)malloc(sizeof(Type) * paramCnt);
        varListHandler(funcDecNode->children[2], param, paramType, 0);
    }

    // Define function symbol in global scope, push old scope into stack and rotate
    Symbol *sym = createFuncSymbol(name, retType, paramCnt, param, paramType);
    Symbol *symbol = resolve(stack, currentScope, name);
    if (symbol != NULL)
    {
        error(4, node->line, "Redefined function", name);
        return;
    }

    define(currentScope, name, sym);
    Scope *scope = createScope(METHOD, name);
    pushScope(stack, currentScope);
    currentScope = scope;

    // Define parameters in function scope
    for (int i = 0; i < paramCnt; i++)
    {
        Symbol *paramSymbol = createVarSymbol(param[i], paramType[i]);
        define(currentScope, param[i], paramSymbol);
    }

    // CompSt
    SyntaxTreeNode *compStNode = node->children[2];
    defListHandler(compStNode->children[1], NULL);
}

// ---------------------------------------------------------------- ⬆ Function Definition

void defListHandler(SyntaxTreeNode *node, Type structType)
{
    // base case
    if (node->childCount == 0)
    {
        return;
    }

    if (structType != NULL)
        varDefHandler(node->children[0], structType, 1);
    else
        varDefHandler(node->children[0], structType, 0);

    // recursive
    defListHandler(node->children[1], structType);
}

Type structDefHandler(SyntaxTreeNode *node, Type recurStructType)
{
    // Define a struct type, anonymous or named
    if (strcmp(node->label, "StructSpecifier") == 0 && strcmp(node->children[1]->label, "OptTag") == 0)
    {
        Type structType = createStructType();
        SyntaxTreeNode *defListNode = node->children[3];
        defListHandler(defListNode, structType);

        char *name;
        if (node->children[1]->childCount == 0)
            name = generate_random_name(5);
        else
            name = node->children[1]->children[0]->lexeme;

        // 错误类型16：结构体的名字与前面定义过的结构体或变量的名字重复。
        if (findSymbol(((structScope *)(currentScope))->table, name) != NULL)
            error(16, node->line, "multiple definition of struct", name);

        Symbol *symbol = createStructSymbol(name, structType);
        define(currentScope, name, symbol);
        return structType;
    }
}

void programHandler()
{
    // def
    stack = createScopeStack();
    currentScope = createScope(GLOBAL, "global");
    Symbol *intSym = createTypeSymbol("int");
    Symbol *floatSym = createTypeSymbol("float");
    define(currentScope, "int", intSym);
    define(currentScope, "float", floatSym);
}

// ---------------------------------------------------------------- ⬆ Struct Definition

varSymbol *expHandler(SyntaxTreeNode *node);
void argsHandler(SyntaxTreeNode *node, Type *types, int argc, int num)
{
    SyntaxTreeNode *arg = node->children[0];
    varSymbol *sym = expHandler(arg);
    Type type = sym->type;
    if (!equal(type, types[num]))
    {
        // Error
        error(9, node->line, "Function is not applicable for arguments", "");
        return;
    }

    if (node->childCount > 1)
    {
        if (num < argc - 1)
            argsHandler(node->children[2], types, argc, num + 1);
        else
            error(9, node->line, "Function is not applicable for arguments", "");
    }
    else
    {
        if (num != argc - 1)
        {
            error(9, node->line, "Function is not applicable for arguments", "");
        }
    }
}

varSymbol *expHandler(SyntaxTreeNode *node)
{
    if (node->childCount == 1)
    {
        // ID
        if (strcmp(node->children[0]->label, "ID") == 0)
        {
            Symbol *symbol = resolveCat(stack, currentScope, node->children[0]->lexeme, VAR);
            if (symbol != NULL)
            {
                return ((varSymbol *)symbol);
            }
            else
            {
                error(1, node->line, "Undefined variable", node->children[0]->lexeme);
                return NULL;
            }
        }
        // INT
        else if (strcmp(node->children[0]->label, "INT") == 0)
        {
            Type type = createBasicType(1);
            varSymbol *symbol = (varSymbol *)createVarSymbol("intTemp", type);
            return symbol;
        }

        // FLOAT
        else if (strcmp(node->children[0]->label, "FLOAT") == 0)
        {
            Type type = createBasicType(2);
            varSymbol *symbol = (varSymbol *)createVarSymbol("floatTemp", type);
            return symbol;
        }
    }

    else
    {
        // Binary calculation
        if (strcmp(node->children[1]->label, "PLUS") == 0 ||
            strcmp(node->children[1]->label, "MINUS") == 0 ||
            strcmp(node->children[1]->label, "MUL") == 0 ||
            strcmp(node->children[1]->label, "DIV") == 0)
        {
            // Operands type matched first
            varSymbol *syml = expHandler(node->children[0]);
            varSymbol *symr = expHandler(node->children[2]);
            if (syml == NULL || symr == NULL)
            {
                // Error

                return NULL;
            }
            if (equalType(syml, symr))
            {
                // Then matched with operator
                if (syml->type->kind == BASIC)
                    return syml;
                else
                {
                    // Error
                    error(7, node->line, "Type mismatched", "");
                    return NULL;
                }
            }
            else
            {
                // Error
                error(7, node->line, "Type mismatched", "");
                return NULL;
            }
        }

        // Logical calculation
        else if (strcmp(node->children[1]->label, "AND") == 0 ||
                 strcmp(node->children[1]->label, "OR") == 0 ||
                 strcmp(node->children[1]->label, "RELOP") == 0)
        {
            // Operands type matched first
            varSymbol *syml = expHandler(node->children[0]);
            varSymbol *symr = expHandler(node->children[2]);
            if (syml == NULL || symr == NULL)
            {
                // Error

                return NULL;
            }
            if (equalType(syml, symr))
            {
                // Then matched with operator
                if (syml->type->kind == BASIC && syml->type->u.basic == 1)
                    return syml;
                else
                {
                    // Error
                    error(7, node->line, "Type mismatched", "");
                    return NULL;
                }
            }
            else
            {
                // Error
                error(7, node->line, "Type mismatched", "");
                return NULL;
            }
        }

        // Assignment
        else if (strcmp(node->children[1]->label, "ASSIGNOP") == 0)
        {
            // Left value check
            SyntaxTreeNode *left = node->children[0];
            if (left->childCount == 1 && strcmp(left->children[0]->label, "ID") != 0)
            {
                // Error
                error(6, node->line, "The left-hand side of an assignment must be a variable", "");
                return NULL;
            }
            else if (left->childCount > 1 && strcmp(left->children[1]->label, "LB") != 0 && strcmp(left->children[1]->label, "DOT") != 0)
            {
                // Error
                error(6, node->line, "The left-hand side of an assignment must be a variable", "");
                return NULL;
            }

            // Type check
            varSymbol *syml = expHandler(node->children[0]);
            varSymbol *symr = expHandler(node->children[2]);
            if (syml == NULL || symr == NULL)
            {
                // Error

                return NULL;
            }
            if (equalType(syml, symr))
            {
                return syml;
            }
            else
            {
                // Error
                error(5, node->line, "Type mismatched for assignment", "");
                return NULL;
            }
        }

        // Unary calculation
        else if (strcmp(node->children[0]->label, "NOT") == 0 ||
                 strcmp(node->children[0]->label, "LP") == 0 ||
                 strcmp(node->children[0]->label, "MINUS") == 0)
        {
            return expHandler(node->children[1]);
        }

        // Array accesss
        else if (strcmp(node->children[1]->label, "LB") == 0)
        {
            varSymbol *symArray = expHandler(node->children[0]);
            varSymbol *symIndex = expHandler(node->children[2]);
            if (symArray == NULL)
            {
                // Error
                return NULL;
            }
            if (symArray->type->kind != ARRAY)
            {
                // Error
                error(10, node->line, "not an array", symArray->name);
                return NULL;
            }
            if (symIndex->type->kind != BASIC || symIndex->type->kind == BASIC && symIndex->type->u.basic == 2)
            {
                // Error
                error(12, node->line, "not an integer", symIndex->name);
                return NULL;
            }
            Type newType = symArray->type->u.array.elem;
            varSymbol *newSym = (varSymbol *)createVarSymbol("temp", newType);
            return newSym;
        }

        // Struct access
        else if (strcmp(node->children[1]->label, "DOT") == 0)
        {
            varSymbol *symStruct = expHandler(node->children[0]);
            if (symStruct == NULL)
            {
                // Error
                return NULL;
            }
            if (symStruct->type->kind != STRUCTURE)
            {
                // Error
                error(13, node->line, "Illegal use of ", ".");
                return NULL;
            }
            char *fieldName = node->children[2]->lexeme;
            Type type = findFieldSymbol(symStruct->type->u.structure, fieldName);
            if (type == NULL)
            {
                // Error
                error(14, node->line, "No such field in structure", fieldName);
                return NULL;
            }
            varSymbol *sym = (varSymbol *)createVarSymbol(fieldName, type);

            return sym;
        }

        // Function call
        else if (strcmp(node->children[0]->label, "ID") == 0)
        {
            Symbol *sym = resolveCat(stack, currentScope, node->children[0]->lexeme, FUNC);
            Symbol *symOther = resolve(stack, currentScope, node->children[0]->lexeme);
            if (sym == NULL)
            {
                // Error
                if (symOther != NULL)
                {
                    // Error
                    error(11, node->line, "not a function", node->children[0]->lexeme);
                    return NULL;
                }
                error(2, node->line, "Undefined function", node->children[0]->lexeme);
                return NULL;
            }

            if (node->childCount > 3)
            {
                int argc = ((funcSymbol *)sym)->paramCount;
                Type *types = ((funcSymbol *)sym)->paramType;
                argsHandler(node->children[2], types, argc, 0);
            }
            char *name = generate_random_name(4);
            Type retType = ((funcSymbol *)sym)->retType;
            varSymbol *retSym = (varSymbol *)createVarSymbol(name, retType);
            return retSym;
        }
    }
}

// ---------------------------------------------------------------- ⬆ EXP

void nodeBeforeHandler(SyntaxTreeNode *node)
{
    // Program
    if (strcmp(node->label, "Program") == 0)
    {
        programHandler();
    }

    // ExtDef
    else if (strcmp(node->label, "ExtDef") == 0)
    {
        SyntaxTreeNode *defNode = node->children[1];
        if (strcmp(defNode->label, "ExtDecList") == 0)
            varDefHandler(node, NULL, 0);

        else if (strcmp(defNode->label, "FunDec") == 0)
            funcDefHandler(node);

        else if (strcmp(defNode->label, "SEMI") == 0)
            structDefHandler(node->children[0]->children[0], NULL);
    }

    // Stmt(CompSt)
    else if (strcmp(node->label, "Stmt") == 0)
    {
        // New nested block
        if (strcmp(node->children[0]->label, "CompSt") == 0)
        {
            Scope *scope = createScope(LOCAL, "local");
            pushScope(stack, currentScope);
            currentScope = scope;
            defListHandler(node->children[0]->children[1], NULL);
        }

        // Return stmt
        else if (strcmp(node->children[0]->label, "RETURN") == 0)
        {
            // Compare Exp type with  previous-difined function's return type
            char *name = ((structSymbol *)currentScope)->name;
            funcSymbol *sym = (funcSymbol *)resolveCat(stack, currentScope, name, FUNC);
            if (sym == NULL)
            {
                return;
            }
            Type retType = sym->retType;
            varSymbol *retSym = expHandler(node->children[1]);
            if (retSym == NULL || !equal(retType, retSym->type))
            {
                error(8, node->line, "Type mismatched for return", "");
            }
        }

        // Exp
        else if (strcmp(node->children[0]->label, "Exp") == 0)
        {
            expHandler(node->children[0]);
        }

        // Control flow
        else if (strcmp(node->children[0]->label, "IF") == 0 ||
                 strcmp(node->children[0]->label, "WHILE") == 0)
        {
            expHandler(node->children[2]);
        }
    }
}

void nodeAfterHandler(SyntaxTreeNode *node)
{
    // Program
    if (strcmp(node->label, "Program") == 0)
    {
        // release  resources
    }

    // ExtDef
    else if (strcmp(node->label, "ExtDef") == 0)
    {
        SyntaxTreeNode *defNode = node->children[1];
        if (strcmp(defNode->label, "ExtDecList") == 0)
        {
        }
        else if (strcmp(defNode->label, "FunDec") == 0)
        {
            if (strcmp(defNode->children[0]->lexeme, ((structScope *)currentScope)->name) == 0)
            {
                // freeScope(currentScope);
                currentScope = popScope(stack);
            }
        }
        else if (strcmp(defNode->label, "SEMI") == 0)
        {
        }
    }

    // Stmt(CompSt)
    else if (strcmp(node->label, "Stmt") == 0)
    {
        if (strcmp(node->children[0]->label, "CompSt") == 0)
        {
            // freeScope(currentScope);
            currentScope = popScope(stack);
        }
    }
}

void dfs(SyntaxTreeNode *node)
{
    // based on Antlr 4
    // Listener
    nodeBeforeHandler(node);
    for (int i = 0; i < node->childCount; i++)
    {
        // [Visiter]
        dfs(node->children[i]);
    }
    nodeAfterHandler(node);
}
