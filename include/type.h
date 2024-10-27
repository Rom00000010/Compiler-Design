// Distinguished from TYPE lexical unit
#ifndef TYPEE
#define TYPEE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type implementation

typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;

char *my_strdup(const char *src) {
    char *dst = malloc(strlen(src) + 1);  // 分配足够的内存
    if (dst == NULL) return NULL;         // 检查malloc是否成功
    strcpy(dst, src);                     // 复制字符串
    return dst;
}

enum Kind
{
    BASIC,
    ARRAY,
    STRUCTURE
};

struct Type_
{
    enum Kind kind;
    union
    {
        // 基本类型
        int basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct
        {
            Type elem;
            int size;
        } array;
        // 结构体类型信息是一个链表
        FieldList structure;
    } u;
};

struct FieldList_
{
    char *name;     // 域的名字
    Type type;      // 域的类型
    FieldList tail; // 下一个域
};

// Int:1, Float:2
Type createBasicType(int basicType)
{
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = BASIC;
    type->u.basic = basicType;
    return type;
}

Type createArrayType(Type elemType, int size)
{
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = ARRAY;
    type->u.array.elem = elemType;
    type->u.array.size = size;
    return type;
}

Type createStructType()
{
    Type type = (Type)malloc(sizeof(struct Type_));
    type->kind = STRUCTURE;
    type->u.structure = NULL;
    return type;
}

void addField(Type type, char *name, Type fieldType, int line)
{
    // 检查是否与结构体中已有字段重名
    FieldList current = type->u.structure;
    while (current != NULL)
    {
        // 错误类型15: 结构体中域名重复定义（指同一结构体中）
        if (strcmp(current->name, name) == 0)
        {
            fprintf(stdout, "Error type 15 at Line %d: multiple definition of field \"%s\"", line, name);
            return;
        }
        current = current->tail;
    }

    // 如果没有重名，进行插入操作 (head insert)
    FieldList field = (FieldList)malloc(sizeof(struct FieldList_));

    field->name = my_strdup(name);

    field->type = fieldType;
    field->tail = type->u.structure; // 头插法
    type->u.structure = field;
}

int equal(Type s1, Type s2);

// Helper function to check if two FieldLists (structure fields) are equal
int equalFieldList(FieldList f1, FieldList f2) {
    while (f1 != NULL && f2 != NULL) {
        // Check if field names are the same
        if (strcmp(f1->name, f2->name) != 0) {
            return 0; // Field names are not equal
        }

        // Recursively check if field types are equal
        if (!equal(f1->type, f2->type)) {
            return 0; // Field types are not equal
        }

        f1 = f1->tail;
        f2 = f2->tail;
    }

    // Both field lists should end at the same time
    return f1 == NULL && f2 == NULL;
}

int equal(Type s1, Type s2) {
    // If both types are null, they are equal
    if (s1 == NULL && s2 == NULL) {
        return 1;
    }

    // If one of them is null, they are not equal
    if (s1 == NULL || s2 == NULL) {
        return 0;
    }

    // Check if the kinds are the same
    if (s1->kind != s2->kind) {
        return 0;
    }

    // Handle different kinds of types
    switch (s1->kind) {
        case BASIC:
            // Basic types are equal if their basic value (int:1 or float:2) is the same
            return s1->u.basic == s2->u.basic;

        case ARRAY:
            // Array types are equal if their element type and size are the same
            return equal(s1->u.array.elem, s2->u.array.elem);

        case STRUCTURE:
            // Structure types are equal if all their fields are the same
            return equalFieldList(s1->u.structure, s2->u.structure);

        default:
            return 0; // Unknown kind, should not happen
    }
}

Type findFieldSymbol(FieldList field, char *fieldName) {
    // Traverse the field list to find a field with the specified name
    while (field != NULL) {
        if (strcmp(field->name, fieldName) == 0) {
            // Field found, return its type
            return field->type;
        }
        field = field->tail;
    }
    // No matching field found, return NULL
    return NULL;
}

#endif