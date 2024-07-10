#ifndef FRX_AST_H
#define FRX_AST_H

#include "token.h"

#include "containers/list.h"

#include "symbols/symbol_table.h"

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

    FRX_AST_TYPE_NEGATED_COMPARISON,
    FRX_AST_TYPE_COMPARISON,

    FRX_AST_TYPE_GREATER_THAN,
    FRX_AST_TYPE_GREATER_THAN_EQUALS,

    FRX_AST_TYPE_LESS_THAN,
    FRX_AST_TYPE_LESS_THAN_EQUALS,

    FRX_AST_TYPE_DEREFERENCE,
    FRX_AST_TYPE_ADDRESS_OF,

    FRX_AST_TYPE_IMPORT_STATEMENT,

    FRX_AST_TYPE_IF_STATEMENT,
    FRX_AST_TYPE_SWITCH_STATEMENT,
    FRX_AST_TYPE_BREAK_STATEMENT,
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

    FRX_AST_TYPE_EXTERN_BLOCK,

    FRX_AST_TYPE_MACRO,
    FRX_AST_TYPE_SIZEOF,

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

//NOTE: Deprecated
typedef struct ASTTypename
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    usize pointer_level;
    AST* array_size;

    ASTNamespaceRef* namespace_ref;
} ASTTypename;

typedef struct ASTVariableDeclaration
{
    ASTTypename* type; //NOTE: Deprecated

    VariableSymbol* variable_symbol;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];
} ASTVariableDeclaration;

typedef struct ASTVariableDefinition
{
    ASTTypename* type; //NOTE: Deprecated

    VariableSymbol* variable_symbol;
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    List array_initialization;

    AST* value;
} ASTVariableDefinition;

typedef struct ASTVariableAssignment
{
    ASTVariable* variable;

    AST* value;
} ASTVariableAssignment;

typedef struct ASTVariable
{
    VariableSymbol* variable_symbol;

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

typedef struct ASTElseIfBlock
{
    AST* condition;
    ASTScope* block;
} ASTElseIfBlock;

typedef struct ASTIfStatement
{
    AST* condition;
    ASTScope* if_block;
    List else_if_blocks;
    ASTScope* else_block;
} ASTIfStatement;

typedef struct ASTSwitchCase
{
    AST* case_expr;
    ASTScope* scope;
} ASTSwitchCase;

typedef struct ASTSwitchStatement
{
    AST* switch_value;
    List cases;
    ASTScope* default_case;
} ASTSwitchStatement;

typedef struct ASTBreakStatement
{
    //NOTE: This struct should not be empty because of the arena allocator
    u8 placeholder;
} ASTBreakStatement;

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
    ASTTypename* type; //NOTE: Deprecated

    FunctionSymbol* function_symbol;

    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    b8 exported;

    ASTParameterList* parameter_list;

    ASTScope* scope;
} ASTFunctionDefinition;

typedef struct ASTFunctionDeclaration
{
    ASTTypename* type; //NOTE: Deprecated

    FunctionSymbol* function_symbol;

    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    ASTParameterList* parameter_list;
} ASTFunctionDeclaration;

typedef struct ASTFunctionCall
{
    FunctionSymbol* function_symbol;

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

    ASTTypename* type; //NOTE: Deprecated

    b8 exported;

    List constants;
} ASTEnumDefinition;

typedef struct ASTStructDefinition
{
    StructSymbol* struct_symbol;

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

typedef struct ASTExternBlock
{
    List function_declarations;
    List struct_definitions;
} ASTExternBlock;

typedef struct ASTMacro
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    AST* value;

    b8 exported;
} ASTMacro;

typedef struct ASTSizeof
{
    char type[FRX_TOKEN_IDENTIFIER_CAPACITY];
} ASTSizeof;

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

void ast_print_switch_statement(const ASTSwitchStatement* switch_statement, usize depth);

void ast_print_break_statement(const ASTBreakStatement* break_statement, usize depth);

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

void ast_print_extern_block(const ASTExternBlock* extern_block, usize depth);

void ast_print_macro(const ASTMacro* macro, usize depth);

void ast_print_sizeof(const ASTSizeof* _sizeof, usize depth);

#endif
