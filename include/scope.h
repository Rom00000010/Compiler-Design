#ifndef SCOPE
#define SCOPE

#include "symTable.h"

typedef enum
{
    GLOBAL,
    METHOD,
    LOCAL,
    STRUCTSCOPE
} ScopeType;

typedef struct Scope
{
    ScopeType type;
} Scope;

typedef struct globalScope
{
    ScopeType type;
    char *name;
    SymbolTable *table;
} globalScope;

typedef struct funcScope
{
    ScopeType type;
    char *name;
    SymbolTable *table;
} funcScope;

typedef struct localScope
{
    ScopeType type;
    char *name;
    SymbolTable *table;
} localScope;

typedef struct structScope
{
    ScopeType type;
    char *name;
    SymbolTable *table;
} structScope;

void *createScope(ScopeType type, char *name)
{
    void *scope = malloc(sizeof(structScope));
    ((structScope *)scope)->type = type;
    ((structScope *)scope)->name = my_strdup(name);
    ((structScope *)scope)->table = createSymbolTable();

    return scope;
}

void freeScope(Scope *scope)
{
    free(((structScope *)scope)->name);
    freeSymbolTable(((structScope *)scope)->table);
}

int define(Scope *scope, char *name, Symbol *sym)
{
    if (findSymbol(((structScope *)scope)->table, name) == NULL)
    {
        insertSymbol(((structScope *)scope)->table, name, sym);
        return 1;
    }
    return 0;
}

#endif