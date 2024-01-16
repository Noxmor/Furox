#ifndef FRX_AST_H
#define FRX_AST_H

#include "token.h"

enum
{
    FRX_AST_TYPE_NOOP = 0,

    FRX_AST_TYPE_COMPOUND,

    FRX_AST_TYPE_NUMBER,

    FRX_AST_TYPE_CHAR_LITERAL,
    FRX_AST_TYPE_STRING_LITERAL,

    FRX_AST_TYPE_VARIABLE_DECLARATION,
    FRX_AST_TYPE_VARIABLE_DEFINITION,
    FRX_AST_TYPE_VARIABLE,

    FRX_AST_TYPE_ADDITION,
    FRX_AST_TYPE_SUBTRACTION,
    FRX_AST_TYPE_MULTIPLICATION,
    FRX_AST_TYPE_DIVISION,

    FRX_AST_TYPE_ARITHMETIC_NEGATION,

    FRX_AST_TYPE_LOGICAL_AND,
    FRX_AST_TYPE_LOGICAL_OR,
    FRX_AST_TYPE_LOGICAL_NEGATION,

    FRX_AST_TYPE_BINARY_AND,
    FRX_AST_TYPE_BINARY_OR,
    FRX_AST_TYPE_BINARY_XOR,
    FRX_AST_TYPE_BINARY_NEGATION,
    FRX_AST_TYPE_BINARY_LEFT_SHIFT,
    FRX_AST_TYPE_BINARY_RIGHT_SHIFT,

    FRX_AST_TYPE_RETURN_STATEMENT,

    FRX_AST_TYPE_PARAMETER_LIST,
    FRX_AST_TYPE_FUNCTION_DEFINITION,
    FRX_AST_TYPE_FUNCTION_CALL,

    FRX_AST_TYPE_SCOPE,

    FRX_AST_TYPE_COUNT
};

typedef u8 ASTType;

typedef struct FunctionDefinitionData
{
    char return_type[FRX_TOKEN_IDENTIFIER_CAPACITY];
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} FunctionDefinitionData;

typedef struct VariableData
{
    char type[FRX_TOKEN_IDENTIFIER_CAPACITY];
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} VariableData;

typedef struct NumberData
{
    usize number;
} NumberData;

typedef struct CharLiteralData
{
    char literal;
} CharLiteralData;

typedef struct StringLiteralData
{
    char literal[FRX_TOKEN_IDENTIFIER_CAPACITY];
} StringLiteralData;

typedef struct AST
{
    ASTType type;
    
    void* data;

    usize children_size;
    usize children_capacity;
    struct AST* children; 
} AST;

const char* ast_type_to_str(ASTType type);

void ast_init(AST* ast, ASTType type);

AST* ast_new_child(AST* parent, ASTType child_type);

void ast_print(const AST* root);

#endif
