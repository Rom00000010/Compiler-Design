#ifndef SYMTABLE
#define SYMTABLE

#include "symbol.h"
#define HASH_TABLE_SIZE 1024

// Symbol table implementation

typedef struct HashNode
{
    char *key;
    Symbol *symbol;
    struct HashNode *next;
} HashNode;

typedef struct
{
    HashNode *buckets[HASH_TABLE_SIZE];
} SymbolTable;

unsigned int hash_pjw(char *str)
{
    unsigned int val = 0, i;
    for (; *str; ++str)
    {
        val = (val << 2) + *str;
        if ((i = val & ~0x3fff))
            val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val % HASH_TABLE_SIZE;
}

SymbolTable *createSymbolTable()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    memset(table->buckets, 0, sizeof(table->buckets));
    return table;
}

void insertSymbol(SymbolTable *table, char *key, Symbol *symbol)
{
    unsigned int index = hash_pjw(key);
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));

    new_node->key = my_strdup(key);
    new_node->symbol = symbol;
    // Head insert
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
}

Symbol *findSymbol(SymbolTable *table, char *key)
{
    unsigned int index = hash_pjw(key);
    for (HashNode *node = table->buckets[index]; node != NULL; node = node->next)
    {
        if (strcmp(key, node->key) == 0)
        {
            return node->symbol;
        }
    }
    return NULL;
}

// Memory management implementation

void freeType(Type type)
{
    if (!type)
        return;
    switch (type->kind)
    {
    case BASIC:
        // 基本类型无额外内存分配
        break;
    case ARRAY:
    {
        // 递归释放元素类型
        freeType(type->u.array.elem);
        break;
    }

    case STRUCTURE:
    {
        // 遍历并释放结构体字段列表
        FieldList field = type->u.structure;
        while (field)
        {
            FieldList temp = field->tail;
            free(field->name);
            freeType(field->type);
            free(field);
            field = temp;
        }
        break;
    }
    }
    free(type);
}

void freeSymbol(Symbol *symbol)
{
    if (!symbol)
        return;
    switch (symbol->category)
    {
    case VAR:
    {
        varSymbol *var = (varSymbol *)symbol;
        free(var->name);
        freeType(var->type);
        free(var);
    }
    break;
    case FUNC:
    {
        funcSymbol *func = (funcSymbol *)symbol;
        free(func->name);
        freeType(func->retType);
        for (int i = 0; i < func->paramCount; i++)
        {
            free(func->param[i]);
            freeType(func->paramType[i]);
        }
        free(func->param);
        free(func->paramType);
        free(func);
    }
    break;
    case TYPESYM:
    {
        typeSymbol *type = (typeSymbol *)symbol;
        free(type->name);
        free(type);
    }
    break;
    }
}

void freeSymbolTable(SymbolTable *table)
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        HashNode *node = table->buckets[i];
        while (node != NULL)
        {
            HashNode *temp = node;
            node = node->next;
            free(temp->key);
            freeSymbol(temp->symbol);
            free(temp);
        }
    }
    free(table);
}

#endif