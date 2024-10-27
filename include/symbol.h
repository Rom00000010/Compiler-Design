#ifndef SYMBOL
#define SYMBOL  

#include "type.h"

// Symbol implementation
typedef enum
{
    VAR,
    FUNC,
    TYPESYM,
    STRUCTSYM
} Category;

typedef struct Symbol
{
    Category category;
} Symbol;

typedef struct varSymbol
{
    Category category;
    char *name;
    Type type;
} varSymbol;

typedef struct funcSymbol
{
    Category category;
    char *name;
    Type retType;
    int paramCount;
    char **param;
    Type *paramType;
} funcSymbol;

typedef struct typeSymbol
{
    Category category;
    char *name;
} typeSymbol;

typedef struct structSymbol
{
    Category category;
    char *name;
    Type type;
} structSymbol;

Symbol* createFuncSymbol(char* name, Type retType, int paramCount, char** param, Type* paramType) {
    funcSymbol* symbol = (funcSymbol*)malloc(sizeof(funcSymbol));

    symbol->category = FUNC;
    symbol->name = my_strdup(name);
    symbol->retType = retType;
    symbol->paramCount = paramCount;

    symbol->param = (char**)malloc(paramCount * sizeof(char*));
    for (int i = 0; i < paramCount; ++i) {
        symbol->param[i] = my_strdup(param[i]);
    }

    symbol->paramType = (Type*)malloc(paramCount * sizeof(Type));
    for (int i = 0; i < paramCount; ++i) {
        symbol->paramType[i] = paramType[i];
    }

    return (Symbol*)symbol;
}

Symbol* createVarSymbol(char* name, Type type) {
    varSymbol* symbol = (varSymbol*)malloc(sizeof(varSymbol));

    symbol->category = VAR;
    symbol->name = my_strdup(name);
    symbol->type = type;

    return (Symbol*)symbol;
}

Symbol* createStructSymbol(char* name, Type type) {
    structSymbol* symbol = (structSymbol*)malloc(sizeof(structSymbol));

    symbol->category = STRUCTSYM;
    symbol->name = my_strdup(name);
    symbol->type = type;
    
    return (Symbol*)symbol;
}

Symbol* createTypeSymbol(char* name) {
    typeSymbol* symbol = (typeSymbol*)malloc(sizeof(typeSymbol));

    symbol->category = TYPESYM;
    symbol->name = my_strdup(name);

    return (Symbol*)symbol;
}

int equalType(varSymbol *s1, varSymbol *s2)
{
    return equal(s1->type, s2->type);
}

#endif