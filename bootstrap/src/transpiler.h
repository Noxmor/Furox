#ifndef FRX_TRANSPILER_H
#define FRX_TRANSPILER_H

#include <stdio.h>

#include "core/core.h"

#include "ast.h"

#include "symbols/symbol_table.h"

enum
{
    FRX_TRANSPILER_MODE_HEADER = 0,
    FRX_TRANSPILER_MODE_SOURCE,

    FRX_TRANSPILER_MODE_COUNT
};

typedef u8 TranspilerMode;

typedef struct Transpiler
{
    FILE* source;
    FILE* header;

    TranspilerMode mode;

    SymbolTable* symbol_table;

    b8 failed;

} Transpiler;

FRX_NO_DISCARD b8 ast_transpile_program(Transpiler* transpiler, const ASTProgram* program, const char* src_filepath);

void ast_transpile(Transpiler* transpiler, const AST* ast);

void ast_transpile_number(Transpiler* transpiler, const ASTNumber* number);

void ast_transpile_char_literal(Transpiler* transpiler, const ASTCharLiteral* char_literal);

void ast_transpile_string_literal(Transpiler* transpiler, const ASTStringLiteral* string_literal);

void ast_transpile_typename(Transpiler* transpiler, const ASTTypename* type);

void ast_transpile_variable_declaration(Transpiler* transpiler, const ASTVariableDeclaration* variable_declaration);

void ast_transpile_variable_definition(Transpiler* transpiler, const ASTVariableDefinition* variable_definition);

void ast_transpile_variable_assignment(Transpiler* transpiler, const ASTVariableAssignment* variable_assignment);

void ast_transpile_variable(Transpiler* transpiler, const ASTVariable* variable);

void ast_transpile_variable_array_access(Transpiler* transpiler, const ASTVariableArrayAccess* variable_array_access);

void ast_transpile_binary_expression(Transpiler* transpiler, const ASTBinaryExpression* binary_expression);

void ast_transpile_unary_expression(Transpiler* transpiler, const ASTUnaryExpression* unary_expression);

void ast_transpile_import_statement(Transpiler* transpiler, const ASTImportStatement* import_statement);

void ast_transpile_if_statement(Transpiler* transpiler, const ASTIfStatement* if_statement);

void ast_transpile_switch_statement(Transpiler* transpiler, const ASTSwitchStatement* switch_statement);

void ast_transpile_break_statement(Transpiler* transpiler, const ASTBreakStatement* break_statement);

void ast_transpile_for_loop(Transpiler* transpiler, const ASTForLoop* for_loop);

void ast_transpile_while_loop(Transpiler* transpiler, const ASTWhileLoop* while_loop);

void ast_transpile_do_while_loop(Transpiler* transpiler, const ASTDoWhileLoop* do_while_loop);

void ast_transpile_return_statement(Transpiler* transpiler, const ASTReturnStatement* return_statement);

void ast_transpile_parameter_list(Transpiler* transpiler, const ASTParameterList* parameter_list);

void ast_transpile_function_definition(Transpiler* transpiler, const ASTFunctionDefinition* function_definition);

void ast_transpile_function_declaration(Transpiler* transpiler, const ASTFunctionDeclaration* function_declaration);

void ast_transpile_function_call(Transpiler* transpiler, const ASTFunctionCall* function_call);

void ast_transpile_scope(Transpiler* transpiler, const ASTScope* scope);

void ast_transpile_enum_definition(Transpiler* transpiler, const ASTEnumDefinition* enum_definition);

void ast_transpile_struct_definition(Transpiler* transpiler, const ASTStructDefinition* struct_definition);

void ast_transpile_namespace(Transpiler* transpiler, const ASTNamespace* namespace);

void ast_transpile_namespace_ref(Transpiler* transpiler, const ASTNamespaceRef* namespace_ref);

void ast_transpile_extern_block(Transpiler* transpiler, const ASTExternBlock* extern_block);

FRX_NO_DISCARD b8 generate_executable(void);

#endif
