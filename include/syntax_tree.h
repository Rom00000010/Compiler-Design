#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SyntaxTreeNode {
    char* label;                       // 节点标签
    int line;                          // 行号
    char* lexeme;                      // 词素，仅对词法单元有效
    struct SyntaxTreeNode** children;  // 子节点数组
    int childCount;                    // 子节点数
} SyntaxTreeNode;

// 创建节点
SyntaxTreeNode* createNode(const char* label, int line, const char* lexeme);

// 添加子节点
void addChildNode(SyntaxTreeNode* parent, SyntaxTreeNode* child);

// 打印语法树
void printTree(SyntaxTreeNode* node, int depth);

// 清理语法树内存
void freeSyntaxTree(SyntaxTreeNode* node);

// 语义分析
void dfs(SyntaxTreeNode* node);

#endif
