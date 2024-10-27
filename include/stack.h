#ifndef STACK
#define STACK

#include "scope.h"

// List implemented stack
typedef struct ScopeNode
{
    Scope *scope;
    struct ScopeNode *next;
} ScopeNode;

typedef struct
{
    ScopeNode *top;
} ScopeStack;

// 初始化栈
ScopeStack *createScopeStack()
{
    ScopeStack *stack = (ScopeStack *)malloc(sizeof(ScopeStack));
    stack->top = NULL;
    return stack;
}

// 入栈操作
void pushScope(ScopeStack *stack, Scope *scope)
{
    ScopeNode *node = (ScopeNode *)malloc(sizeof(ScopeNode));
    node->scope = scope;
    node->next = stack->top;
    stack->top = node;
}

// 出栈操作, 回到上一级作用域, 或者上一级的邻接的某个子作用域
Scope *popScope(ScopeStack *stack)
{
    if (stack->top == NULL)
    {
        return NULL;
    }
    ScopeNode *topNode = stack->top;
    stack->top = topNode->next;
    Scope *scope = topNode->scope;
    free(topNode);
    return scope;
}

// 查看栈顶元素
Scope *peekScope(ScopeStack *stack)
{
    if (stack->top == NULL)
    {
        return NULL;
    }
    return stack->top->scope;
}

Symbol *findSymbolInScopes(ScopeStack *stack, char *name)
{
    ScopeNode *node = stack->top;
    while (node != NULL)
    {
        Symbol *symbol = findSymbol(((structScope *)(node->scope))->table, name);
        if (symbol != NULL)
        {
            return symbol;
        }
        node = node->next;
    }
    return NULL; // Symbol not found in any scopes
}

Symbol *findSymbolInScopesCat(ScopeStack *stack, char *name, Category cat)
{
    ScopeNode *node = stack->top;
    while (node != NULL)
    {
        Symbol *symbol = findSymbol(((structScope *)(node->scope))->table, name);
        if (symbol != NULL && ((varSymbol *)symbol)->category == cat)
        {
            return symbol;
        }
        node = node->next;
    }
    return NULL; // Symbol not found in any scopes
}

Symbol *resolve(ScopeStack *stack, Scope *currentScope, char *name)
{
    Symbol *symbol = findSymbol(((structScope *)(currentScope))->table, name);
    if (symbol == NULL)
    {
        symbol = findSymbolInScopes(stack, name);
    }
    return symbol;
}

Symbol *resolveCat(ScopeStack *stack, Scope *currentScope, char *name, Category cat)
{
    Symbol *symbol = findSymbol(((structScope *)(currentScope))->table, name);
    if (symbol == NULL || ((varSymbol *)symbol)->category != cat)
    {
        symbol = findSymbolInScopesCat(stack, name, cat);
    }
    return symbol;
}

// 释放栈内存
void freeScopeStack(ScopeStack *stack)
{
    while (stack->top != NULL)
    {
        Scope *scope = popScope(stack);
        freeScope(scope);
    }
    free(stack);
}

#endif