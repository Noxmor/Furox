#ifndef FRX_AST_H
#define FRX_AST_H

#include "token.h"

#include "containers/list.h"

enum
{
    FRX_AST_TYPE_NOOP = 0,

    FRX_AST_TYPE_PROGRAM,

    FRX_AST_TYPE_COMPOUND,

    FRX_AST_TYPE_NUMBER,

    FRX_AST_TYPE_CHAR_LITERAL,
    FRX_AST_TYPE_STRING_LITERAL,

    FRX_AST_TYPE_TYPE,

    FRX_AST_TYPE_VARIABLE_DECLARATION,
    FRX_AST_TYPE_VARIABLE_DEFINITION,
    FRX_AST_TYPE_VARIABLE_ASSIGNMENT,
    FRX_AST_TYPE_VARIABLE,
    FRX_AST_TYPE_VARIABLE_ARRAY_ACCESS,

    FRX_AST_TYPE_UNARY_EXPRESSION,
    FRX_AST_TYPE_BINARY_EXPRESSION,

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

    FRX_AST_TYPE_DEREFERENCE,
    FRX_AST_TYPE_ADDRESS_OF,

    FRX_AST_TYPE_IMPORT_STATEMENT,

    FRX_AST_TYPE_IF_STATEMENT,
    FRX_AST_TYPE_FOR_LOOP,
    FRX_AST_TYPE_WHILE_LOOP,
    FRX_AST_TYPE_DO_WHILE_LOOP,
    FRX_AST_TYPE_RETURN_STATEMENT,

    FRX_AST_TYPE_PARAMETER_LIST,
    FRX_AST_TYPE_FUNCTION_DEFINITION,
    FRX_AST_TYPE_FUNCTION_DECLARATION,
    FRX_AST_TYPE_FUNCTION_CALL,

    FRX_AST_TYPE_SCOPE,

    FRX_AST_TYPE_ENUM_DEFINITION,
    FRX_AST_TYPE_STRUCT_DEFINITION,

    FRX_AST_TYPE_NAMESPACE,
    FRX_AST_TYPE_NAMESPACE_REF,

    FRX_AST_TYPE_MODULE_DEFINITION,
    FRX_AST_TYPE_MODULE_IMPLEMENTATION,

    FRX_AST_TYPE_EXTERN_BLOCK,

    FRX_AST_TYPE_COUNT
};

typedef u8 ASTType;

const char* ast_type_to_str(ASTType type);

typedef struct AST
{
    ASTType type;

    void* node;
} AST;

typedef struct ASTNamespaceRef ASTNamespaceRef;
typedef struct ASTVariable ASTVariable;
typedef struct ASTScope ASTScope;

typedef struct ASTProgram
{
    List top_level_definitions;
} ASTProgram;

typedef struct ASTNumber
{
    usize number;
} ASTNumber;

typedef struct ASTCharLiteral
{
    char literal[3];
} ASTCharLiteral;

typedef struct ASTStringLiteral
{
    char literal[FRX_TOKEN_IDENTIFIER_CAPACITY];
} ASTStringLiteral;

typedef struct ASTTypename
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    usize pointer_level;
    AST* array_size;

    ASTNamespaceRef* namespace_ref;
} ASTTypename;

typedef struct ASTVariableDeclaration
{
    ASTTypename* type;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} ASTVariableDeclaration;

typedef struct ASTVariableDefinition
{
    ASTTypename* type;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    AST* value;
} ASTVariableDefinition;

typedef struct ASTVariableAssignment
{
    ASTVariable* variable;

    AST* value;
} ASTVariableAssignment;

typedef struct ASTVariable
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 is_pointer;

    AST* array_index;

    ASTVariable* next;
} ASTVariable;

typedef struct ASTVariableArrayAccess
{
    ASTVariable* variable;
    AST* index;
    AST* value;
} ASTVariableArrayAccess;

typedef struct ASTBinaryExpression
{
    ASTType type;
    AST* left;
    AST* right;
} ASTBinaryExpression;

typedef struct ASTUnaryExpression
{
    ASTType type;
    AST* operand;
} ASTUnaryExpression;

typedef struct ASTImportStatement
{
    char filepath[FRX_TOKEN_IDENTIFIER_CAPACITY];
} ASTImportStatement;

typedef struct ASTIfStatement
{
    AST* condition;
    ASTScope* if_block;
    ASTScope* else_block;
} ASTIfStatement;

typedef struct ASTForLoop
{
    AST* expression;
    AST* condition;
    AST* increment;

    ASTScope* scope;
} ASTForLoop;

typedef struct ASTWhileLoop
{
    AST* condition;
    ASTScope* scope;
} ASTWhileLoop;

typedef struct ASTDoWhileLoop
{
    ASTScope* scope;
    AST* condition;
} ASTDoWhileLoop;

typedef struct ASTReturnStatement
{
    AST* value;
} ASTReturnStatement;

typedef struct ASTParameterList
{
    b8 is_variadic;

    List parameters;
} ASTParameterList;

typedef struct ASTFunctionDefinition
{
    ASTTypename* type;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 exported;

    ASTParameterList* parameter_list;

    ASTScope* scope;
} ASTFunctionDefinition;

typedef struct ASTFunctionDeclaration
{
    ASTTypename* type;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    ASTParameterList* parameter_list;
} ASTFunctionDeclaration;

typedef struct ASTFunctionCall
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    ASTNamespaceRef* namespace_ref;

    List arguments;
} ASTFunctionCall;

typedef struct ASTScope
{
    List statements;
} ASTScope;

typedef struct ASTEnumDefinition
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    ASTTypename* type;

    b8 exported;

    List constants;
} ASTEnumDefinition;

typedef struct ASTStructDefinition
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 exported;

    List fields;
} ASTStructDefinition;

typedef struct ASTNamespace
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    List top_level_definitions;
} ASTNamespace;

typedef struct ASTNamespaceRef
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    ASTNamespaceRef* next;
} ASTNamespaceRef;

typedef struct ASTModuleDefinition
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 exported;

    List function_declarations;
} ASTModuleDefinition;

typedef struct ASTModuleImplementation
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    List function_definitions;
} ASTModuleImplementation;

typedef struct ASTExternBlock
{
    List function_declarations;
    List struct_definitions;
} ASTExternBlock;

void ast_print(const AST* ast, usize depth);

void ast_print_noop(usize depth);

void ast_print_program(const ASTProgram* program);

void ast_print_number(const ASTNumber* number, usize depth);

void ast_print_char_literal(const ASTCharLiteral* char_literal, usize depth);

void ast_print_string_literal(const ASTStringLiteral* string_literal, usize depth);

void ast_print_typename(const ASTTypename* type, usize depth);

void ast_print_variable_declaration(const ASTVariableDeclaration* variable_declaration, usize depth);

void ast_print_variable_definition(const ASTVariableDefinition* variable_definition, usize depth);

void ast_print_variable_assignment(const ASTVariableAssignment* variable_assignment, usize depth);

void ast_print_variable(const ASTVariable* variable, usize depth);

void ast_print_variable_array_access(const ASTVariableArrayAccess* variable_array_access, usize depth);

void ast_print_binary_expression(const ASTBinaryExpression* binary_expression, usize depth);

void ast_print_unary_expression(const ASTUnaryExpression* unary_expression, usize depth);

void ast_print_import_statement(const ASTImportStatement* import_statement, usize depth);

void ast_print_if_statement(const ASTIfStatement* if_statement, usize depth);

void ast_print_for_loop(const ASTForLoop* for_loop, usize depth);

void ast_print_while_loop(const ASTWhileLoop* while_loop, usize depth);

void ast_print_do_while_loop(const ASTDoWhileLoop* do_while_loop, usize depth);

void ast_print_return_statement(const ASTReturnStatement* return_statement, usize depth);

void ast_print_parameter_list(const ASTParameterList* parameter_list, usize depth);

void ast_print_function_definition(const ASTFunctionDefinition* function_definition, usize depth);

void ast_print_function_declaration(const ASTFunctionDeclaration* function_declaration, usize depth);

void ast_print_function_call(const ASTFunctionCall* function_call, usize depth);

void ast_print_scope(const ASTScope* scope, usize depth);

void ast_print_enum_definition(const ASTEnumDefinition* enum_definition, usize depth);

void ast_print_struct_definition(const ASTStructDefinition* struct_definition, usize depth);

void ast_print_namespace(const ASTNamespace* namespace, usize depth);

void ast_print_namespace_ref(const ASTNamespaceRef* namespace_ref, usize depth);

void ast_print_module_definition(const ASTModuleDefinition* module_definition, usize depth);

void ast_print_module_implementation(const ASTModuleImplementation* module_implementation, usize depth);

void ast_print_extern_block(const ASTExternBlock* extern_block, usize depth);

#endif
