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

    FRX_AST_TYPE_TYPE,

    FRX_AST_TYPE_VARIABLE_DECLARATION,
    FRX_AST_TYPE_VARIABLE_DEFINITION,
    FRX_AST_TYPE_VARIABLE_ASSIGNMENT,
    FRX_AST_TYPE_VARIABLE,

    FRX_AST_TYPE_ADDITION,
    FRX_AST_TYPE_SUBTRACTION,
    FRX_AST_TYPE_MULTIPLICATION,
    FRX_AST_TYPE_DIVISION,
    FRX_AST_TYPE_MODULO,

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

    FRX_AST_TYPE_COMPARISON,

    FRX_AST_TYPE_GREATER_THAN,
    FRX_AST_TYPE_GREATER_THAN_EQUALS,

    FRX_AST_TYPE_LESS_THAN,
    FRX_AST_TYPE_LESS_THAN_EQUALS,

    FRX_AST_TYPE_RETURN_STATEMENT,

    FRX_AST_TYPE_PARAMETER_LIST,
    FRX_AST_TYPE_FUNCTION_DEFINITION,
    FRX_AST_TYPE_FUNCTION_DECLARATION,
    FRX_AST_TYPE_FUNCTION_CALL,

    FRX_AST_TYPE_SCOPE,

    FRX_AST_TYPE_STRUCT_DEFINITION,

    FRX_AST_TYPE_NAMESPACE,
    FRX_AST_TYPE_NAMESPACE_REF,

    FRX_AST_TYPE_EXTERN_BLOCK,

    FRX_AST_TYPE_COUNT
};

typedef u8 ASTType;

typedef struct TypeData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    usize pointer_level;
} TypeData;

typedef struct VariableData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} VariableData;

typedef struct NumberData
{
    usize number;
} NumberData;

typedef struct CharLiteralData
{
    char literal[3];
} CharLiteralData;

typedef struct StringLiteralData
{
    char literal[FRX_TOKEN_IDENTIFIER_CAPACITY];
} StringLiteralData;

typedef struct FunctionDefinitionData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 is_variadic;
} FunctionDefinitionData;

typedef struct FunctionDeclarationData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 is_variadic;
} FunctionDeclarationData;

typedef struct FunctionCallData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 is_statement;
} FunctionCallData;

typedef struct StructDefinitionData
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} StructDefinitionData;

typedef struct NamespaceData
{
    char namespace[FRX_TOKEN_IDENTIFIER_CAPACITY];
} NamespaceData;

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

AST* ast_new_child(AST* parent);

void ast_print(const AST* root);

#endif
