#include "ast.h"

#include "core/assert.h"
#include "core/memory.h"

const char* ast_type_to_str(ASTType type)
{
    FRX_ASSERT(type < FRX_AST_TYPE_COUNT);

    switch(type)
    {
        case FRX_AST_TYPE_NOOP: return "noop";

        case FRX_AST_TYPE_PROGRAM: return "program";

        case FRX_AST_TYPE_COMPOUND: return "compound";

        case FRX_AST_TYPE_NUMBER: return "number";

        case FRX_AST_TYPE_CHAR_LITERAL: return "char-literal";
        case FRX_AST_TYPE_STRING_LITERAL: return "string-literal";

        case FRX_AST_TYPE_TYPE: return "type";

        case FRX_AST_TYPE_VARIABLE_DECLARATION: return "variable-declaration";
        case FRX_AST_TYPE_VARIABLE_DEFINITION: return "variable-definition";
        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT: return "variable-assignment";
        case FRX_AST_TYPE_VARIABLE: return "variable";
        case FRX_AST_TYPE_VARIABLE_ARRAY_ACCESS: return "variable-array-access";

        case FRX_AST_TYPE_UNARY_EXPRESSION: return "unary-expression";
        case FRX_AST_TYPE_BINARY_EXPRESSION: return "binary-expression";

        case FRX_AST_TYPE_ADDITION: return "addition";
        case FRX_AST_TYPE_SUBTRACTION: return "subtraction";
        case FRX_AST_TYPE_MULTIPLICATION: return "multiplication";
        case FRX_AST_TYPE_DIVISION: return "division";
        case FRX_AST_TYPE_MODULO: return "modulo";

        case FRX_AST_TYPE_ARITHMETIC_NEGATION: return "arithmetic-negation";

        case FRX_AST_TYPE_LOGICAL_AND: return "logical-and";
        case FRX_AST_TYPE_LOGICAL_OR: return "logical-or";
        case FRX_AST_TYPE_LOGICAL_NEGATION: return "logical-negation";

        case FRX_AST_TYPE_BINARY_AND: return "binary-and";
        case FRX_AST_TYPE_BINARY_OR: return "binary-or";
        case FRX_AST_TYPE_BINARY_XOR: return "binary-xor";
        case FRX_AST_TYPE_BINARY_NEGATION: return "binary-negation";
        case FRX_AST_TYPE_BINARY_LEFT_SHIFT: return "binary-left-shift";
        case FRX_AST_TYPE_BINARY_RIGHT_SHIFT: return "binary-right-shift";

        case FRX_AST_TYPE_COMPARISON: return "comparison";

        case FRX_AST_TYPE_GREATER_THAN: return "greater-than";
        case FRX_AST_TYPE_GREATER_THAN_EQUALS: return "greater-than-equals";

        case FRX_AST_TYPE_LESS_THAN: return "less-than";
        case FRX_AST_TYPE_LESS_THAN_EQUALS: return "less-than-equals";

        case FRX_AST_TYPE_DEREFERENCE: return "dereference";
        case FRX_AST_TYPE_ADDRESS_OF: return "address-of";

        case FRX_AST_TYPE_IMPORT_STATEMENT: return "import-statement";

        case FRX_AST_TYPE_IF_STATEMENT: return "if-statement";
        case FRX_AST_TYPE_FOR_LOOP: return "for-loop";
        case FRX_AST_TYPE_WHILE_LOOP: return "while-loop";
        case FRX_AST_TYPE_DO_WHILE_LOOP: return "do-while-loop";
        case FRX_AST_TYPE_RETURN_STATEMENT: return "return-statement";

        case FRX_AST_TYPE_PARAMETER_LIST: return "parameter-list";
        case FRX_AST_TYPE_FUNCTION_DEFINITION: return "function-definition";
        case FRX_AST_TYPE_FUNCTION_DECLARATION: return "function-declaration";
        case FRX_AST_TYPE_FUNCTION_CALL: return "function-call";

        case FRX_AST_TYPE_SCOPE: return "scope";

        case FRX_AST_TYPE_ENUM_DEFINITION: return "enum-definition";
        case FRX_AST_TYPE_STRUCT_DEFINITION: return "struct-definition";

        case FRX_AST_TYPE_NAMESPACE: return "namespace";
        case FRX_AST_TYPE_NAMESPACE_REF: return "namespace-reference";

        case FRX_AST_TYPE_MODULE_DEFINITION: return "module-definition";
        case FRX_AST_TYPE_MODULE_IMPLEMENTATION: return "module-implementation";

        case FRX_AST_TYPE_EXTERN_BLOCK: return "extern-block";

        default: FRX_ASSERT(FRX_FALSE);
    }

    return "Unknown";
}

#define FRX_AST_PRINT_RECURSION_LIMIT 128
static u8 recursion_buffer[FRX_AST_PRINT_RECURSION_LIMIT];

static void print_recursion_buffer(usize depth)
{
    FRX_ASSERT(depth < FRX_AST_PRINT_RECURSION_LIMIT);

    for(usize i = 0; i < depth; ++i)
    {
        if(i < depth - 1)
            printf("%c   ", recursion_buffer[i] ? '|' : ' ');
        else
            printf("+---");
    }
}

static void print_depth(usize depth)
{
    FRX_ASSERT(depth < FRX_AST_PRINT_RECURSION_LIMIT);

    for(usize i = 0; i < depth; ++i)
    {
        printf("%c   ", recursion_buffer[i] ? '|' : ' ');
    }

    printf("|\n");
}

void ast_print(const AST* ast, usize depth)
{
    FRX_ASSERT(ast != NULL);

    switch(ast->type)
    {
        case FRX_AST_TYPE_NOOP: ast_print_noop(depth); break;

        case FRX_AST_TYPE_NUMBER: ast_print_number(ast->node, depth); break;
        case FRX_AST_TYPE_CHAR_LITERAL: ast_print_char_literal(ast->node, depth); break;
        case FRX_AST_TYPE_STRING_LITERAL: ast_print_string_literal(ast->node, depth); break;

        case FRX_AST_TYPE_VARIABLE_DECLARATION: ast_print_variable_declaration(ast->node, depth); break;
        case FRX_AST_TYPE_VARIABLE_DEFINITION: ast_print_variable_definition(ast->node, depth); break;
        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT: ast_print_variable_assignment(ast->node, depth); break;
        case FRX_AST_TYPE_VARIABLE: ast_print_variable(ast->node, depth); break;
        case FRX_AST_TYPE_VARIABLE_ARRAY_ACCESS: ast_print_variable_array_access(ast->node, depth); break;

        case FRX_AST_TYPE_BINARY_EXPRESSION: ast_print_binary_expression(ast->node, depth); break;
        case FRX_AST_TYPE_UNARY_EXPRESSION: ast_print_unary_expression(ast->node, depth); break;

        case FRX_AST_TYPE_IMPORT_STATEMENT: ast_print_import_statement(ast->node, depth); break;

        case FRX_AST_TYPE_IF_STATEMENT: ast_print_if_statement(ast->node, depth); break;
        case FRX_AST_TYPE_FOR_LOOP: ast_print_for_loop(ast->node, depth); break;
        case FRX_AST_TYPE_WHILE_LOOP: ast_print_while_loop(ast->node, depth); break;
        case FRX_AST_TYPE_DO_WHILE_LOOP: ast_print_do_while_loop(ast->node, depth); break;

        case FRX_AST_TYPE_RETURN_STATEMENT: ast_print_return_statement(ast->node, depth); break;

        case FRX_AST_TYPE_FUNCTION_DEFINITION: ast_print_function_definition(ast->node, depth); break;
        case FRX_AST_TYPE_FUNCTION_CALL: ast_print_function_call(ast->node, depth); break;
        case FRX_AST_TYPE_ENUM_DEFINITION: ast_print_enum_definition(ast->node, depth); break;
        case FRX_AST_TYPE_STRUCT_DEFINITION: ast_print_struct_definition(ast->node, depth); break;
        case FRX_AST_TYPE_NAMESPACE: ast_print_namespace(ast->node, depth); break;
        case FRX_AST_TYPE_MODULE_DEFINITION: ast_print_module_definition(ast->node, depth); break;
        case FRX_AST_TYPE_MODULE_IMPLEMENTATION: ast_print_module_implementation(ast->node, depth); break;
        case FRX_AST_TYPE_EXTERN_BLOCK: ast_print_extern_block(ast->node, depth); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void ast_print_noop(usize depth)
{
    print_recursion_buffer(depth);

    printf("noop\n");
}

void ast_print_program(const ASTProgram* program)
{
    FRX_ASSERT(program != NULL);

    usize depth = 0;

    print_recursion_buffer(depth);

    printf("program\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&program->top_level_definitions);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        printf("|\n");

        ast_print(list_get(&program->top_level_definitions, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_number(const ASTNumber* number, usize depth)
{
    FRX_ASSERT(number != NULL);

    print_recursion_buffer(depth);

    printf("number (%zu)\n", number->number);
}

void ast_print_char_literal(const ASTCharLiteral* char_literal, usize depth)
{
    FRX_ASSERT(char_literal != NULL);

    print_recursion_buffer(depth);

    printf("char-literal ('%s')\n", char_literal->literal);
}

void ast_print_string_literal(const ASTStringLiteral* string_literal, usize depth)
{
    FRX_ASSERT(string_literal != NULL);

    print_recursion_buffer(depth);

    printf("string-literal (\"%s\")\n", string_literal->literal);
}

void ast_print_typename(const ASTTypename* type, usize depth)
{
    FRX_ASSERT(type != NULL);

    print_recursion_buffer(depth);

    printf("type (%s)\n", type->name);

    if(type->array_size != NULL)
    {
        recursion_buffer[depth] = 1;

        print_depth(depth);

        ast_print(type->array_size, depth + 1);

        recursion_buffer[depth] = 0;
    }

    if(type->namespace_ref != NULL)
    {
        print_depth(depth);

        ast_print_namespace_ref(type->namespace_ref, depth + 1);
    }
}

void ast_print_variable_declaration(const ASTVariableDeclaration* variable_declaration, usize depth)
{
    FRX_ASSERT(variable_declaration != NULL);

    print_recursion_buffer(depth);

    printf("variable-declaration (%s)\n", variable_declaration->name);

    ast_print_typename(variable_declaration->type, depth + 1);
}

void ast_print_variable_definition(const ASTVariableDefinition* variable_definition, usize depth)
{
    FRX_ASSERT(variable_definition != NULL);

    print_recursion_buffer(depth);

    printf("variable-definition (%s)\n", variable_definition->name);

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_typename(variable_definition->type, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print(variable_definition->value, depth + 1);
}

void ast_print_variable_assignment(const ASTVariableAssignment* variable_assignment, usize depth)
{
    FRX_ASSERT(variable_assignment != NULL);

    print_recursion_buffer(depth);

    printf("variable-assignment\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_variable(variable_assignment->variable, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print(variable_assignment->value, depth + 1);
}

void ast_print_variable(const ASTVariable* variable, usize depth)
{
    FRX_ASSERT(variable != NULL);

    print_recursion_buffer(depth);

    printf("variable (%s)\n", variable->name);

    if(variable->array_index != NULL)
    {
        recursion_buffer[depth] = 1;

        print_depth(depth);

        ast_print(variable->array_index, depth + 1);

        recursion_buffer[depth] = 0;
    }

    if(variable->next != NULL)
    {
        print_depth(depth);

        ast_print_variable(variable->next, depth + 1);
    }
}

void ast_print_variable_array_access(const ASTVariableArrayAccess* variable_array_access, usize depth)
{
    FRX_ASSERT(variable_array_access != NULL);

    print_recursion_buffer(depth);

    printf("variable-array-access\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_variable(variable_array_access->variable, depth + 1);

    print_depth(depth);

    ast_print(variable_array_access->index, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print(variable_array_access->value, depth + 1);
}

void ast_print_binary_expression(const ASTBinaryExpression* binary_expression, usize depth)
{
    FRX_ASSERT(binary_expression != NULL);

    print_recursion_buffer(depth);

    printf("binary-expression (%s)\n", ast_type_to_str(binary_expression->type));

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print(binary_expression->left, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print(binary_expression->right, depth + 1);
}

void ast_print_unary_expression(const ASTUnaryExpression* unary_expression, usize depth)
{
    FRX_ASSERT(unary_expression != NULL);

    print_recursion_buffer(depth);

    printf("unary-expression (%s)\n", ast_type_to_str(unary_expression->type));

    print_depth(depth);

    ast_print(unary_expression->operand, depth + 1);
}

void ast_print_import_statement(const ASTImportStatement* import_statement, usize depth)
{
    FRX_ASSERT(import_statement != NULL);

    print_recursion_buffer(depth);

    printf("import-statement (\"%s\")\n", import_statement->filepath);
}

void ast_print_if_statement(const ASTIfStatement* if_statement, usize depth)
{
    FRX_ASSERT(if_statement != NULL);

    print_recursion_buffer(depth);

    printf("if-statement\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print(if_statement->condition, depth + 1);

    if(if_statement->else_block != NULL)
    {
        print_depth(depth);

        ast_print_scope(if_statement->if_block, depth + 1);

        recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_scope(if_statement->else_block, depth + 1);
    }
    else
    {
        recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_scope(if_statement->if_block, depth + 1);
    }
}

void ast_print_for_loop(const ASTForLoop* for_loop, usize depth)
{
    FRX_ASSERT(for_loop != NULL);

    print_recursion_buffer(depth);

    printf("for-loop\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print(for_loop->expression, depth + 1);

    print_depth(depth);

    ast_print(for_loop->condition, depth + 1);

    print_depth(depth);

    ast_print(for_loop->increment, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print_scope(for_loop->scope, depth + 1);
}

void ast_print_while_loop(const ASTWhileLoop* while_loop, usize depth)
{
    FRX_ASSERT(while_loop != NULL);

    print_recursion_buffer(depth);

    printf("while-loop\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print(while_loop->condition, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print_scope(while_loop->scope, depth + 1);
}

void ast_print_do_while_loop(const ASTDoWhileLoop* do_while_loop, usize depth)
{
    FRX_ASSERT(do_while_loop != NULL);

    print_recursion_buffer(depth);

    printf("do-while-loop\n");

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_scope(do_while_loop->scope, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print(do_while_loop->condition, depth + 1);
}

void ast_print_return_statement(const ASTReturnStatement* return_statement, usize depth)
{
    FRX_ASSERT(return_statement != NULL);

    print_recursion_buffer(depth);

    printf("return-statement\n");

    if(return_statement->value == NULL)
        return;

    print_depth(depth);

    ast_print(return_statement->value, depth + 1);
}

void ast_print_parameter_list(const ASTParameterList* parameter_list, usize depth)
{
    FRX_ASSERT(parameter_list != NULL);

    print_recursion_buffer(depth);

    printf("parameter-list\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&parameter_list->parameters);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_variable_declaration(list_get(&parameter_list->parameters, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_function_definition(const ASTFunctionDefinition* function_definition, usize depth)
{
    FRX_ASSERT(function_definition != NULL);

    print_recursion_buffer(depth);

    printf("function-definition (%s)\n", function_definition->name);

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_typename(function_definition->type, depth + 1);

    print_depth(depth);

    ast_print_parameter_list(function_definition->parameter_list, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print_scope(function_definition->scope, depth + 1);
}

void ast_print_function_declaration(const ASTFunctionDeclaration* function_declaration, usize depth)
{
    FRX_ASSERT(function_declaration != NULL);

    print_recursion_buffer(depth);

    printf("function-declaration (%s)\n", function_declaration->name);

    recursion_buffer[depth] = 1;

    print_depth(depth);

    ast_print_typename(function_declaration->type, depth + 1);

    recursion_buffer[depth] = 0;

    print_depth(depth);

    ast_print_parameter_list(function_declaration->parameter_list, depth + 1);
}

void ast_print_function_call(const ASTFunctionCall* function_call, usize depth)
{
    FRX_ASSERT(function_call != NULL);

    print_recursion_buffer(depth);

    printf("function-call (%s)\n", function_call->name);

    recursion_buffer[depth] = 1;

    if(function_call->namespace_ref != NULL)
    {
        print_depth(depth);

        ast_print_namespace_ref(function_call->namespace_ref, depth + 1);
    }

    usize size = list_size(&function_call->arguments);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print(list_get(&function_call->arguments, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_scope(const ASTScope* scope, usize depth)
{
    FRX_ASSERT(scope != NULL);

    print_recursion_buffer(depth);

    printf("scope\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&scope->statements);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print(list_get(&scope->statements, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_enum_definition(const ASTEnumDefinition* enum_definition, usize depth)
{
    FRX_ASSERT(enum_definition != NULL);

    print_recursion_buffer(depth);

    printf("enum-definition (%s)\n", enum_definition->name);

    recursion_buffer[depth] = 1;

    usize size = list_size(&enum_definition->constants);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print(list_get(&enum_definition->constants, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_struct_definition(const ASTStructDefinition* struct_definition, usize depth)
{
    FRX_ASSERT(struct_definition != NULL);

    print_recursion_buffer(depth);

    printf("struct-definition (%s)\n", struct_definition->name);

    recursion_buffer[depth] = 1;

    usize size = list_size(&struct_definition->fields);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_variable_declaration(list_get(&struct_definition->fields, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_namespace(const ASTNamespace* namespace, usize depth)
{
    FRX_ASSERT(namespace != NULL);

    print_recursion_buffer(depth);

    printf("namespace (%s)\n", namespace->name);

    recursion_buffer[depth] = 1;

    usize size = list_size(&namespace->top_level_definitions);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print(list_get(&namespace->top_level_definitions, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_namespace_ref(const ASTNamespaceRef* namespace_ref, usize depth)
{
    FRX_ASSERT(namespace_ref != NULL);

    print_recursion_buffer(depth);

    printf("namespace-ref (%s)\n", namespace_ref->name);

    if(namespace_ref->next == NULL)
        return;

    print_depth(depth);

    ast_print_namespace_ref(namespace_ref->next, depth + 1);
}

void ast_print_module_definition(const ASTModuleDefinition* module_definition, usize depth)
{
    FRX_ASSERT(module_definition != NULL);

    print_recursion_buffer(depth);

    printf("module-definition\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&module_definition->function_declarations);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_function_declaration(list_get(&module_definition->function_declarations, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_module_implementation(const ASTModuleImplementation* module_implementation, usize depth)
{
    FRX_ASSERT(module_implementation != NULL);

    print_recursion_buffer(depth);

    printf("module-implementation\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&module_implementation->function_definitions);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_function_definition(list_get(&module_implementation->function_definitions, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}

void ast_print_extern_block(const ASTExternBlock* extern_block, usize depth)
{
    FRX_ASSERT(extern_block != NULL);

    print_recursion_buffer(depth);

    printf("extern_block\n");

    recursion_buffer[depth] = 1;

    usize size = list_size(&extern_block->struct_definitions);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_struct_definition(list_get(&extern_block->struct_definitions, i), depth + 1);
    }

    size = list_size(&extern_block->function_declarations);
    for(usize i = 0; i < size; ++i)
    {
        if(i == size - 1)
            recursion_buffer[depth] = 0;

        print_depth(depth);

        ast_print_function_declaration(list_get(&extern_block->function_declarations, i), depth + 1);
    }

    recursion_buffer[depth] = 0;
}
