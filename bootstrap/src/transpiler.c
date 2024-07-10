#include "transpiler.h"

#include <ctype.h>
#include <string.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "ast.h"
#include "core/assert.h"
#include "core/core.h"
#include "core/memory.h"
#include "symbols/type_category.h"

#define FRX_TRANSPILER_WRITE(transpiler, format, ...) do { if(fprintf(transpiler->mode == FRX_TRANSPILER_MODE_HEADER ? transpiler->header : transpiler->source, format, ##__VA_ARGS__) < 0) transpiler->failed = FRX_TRUE; } while(FRX_FALSE)
#define FRX_TRANSPILER_WRITE_HEADER(transpiler, format, ...) do { if(fprintf(transpiler->header, format, ##__VA_ARGS__) < 0) transpiler->failed = FRX_TRUE; } while(FRX_FALSE)
#define FRX_TRANSPILER_WRITE_SOURCE(transpiler, format, ...) do { if(fprintf(transpiler->source, format, ##__VA_ARGS__) < 0) transpiler->failed = FRX_TRUE; } while(FRX_FALSE)

typedef struct TranspilerInfo
{
    char** c_filepaths;
    usize c_filepaths_size;
    usize c_filepaths_capacity;
} TranspilerInfo;

static TranspilerInfo transpiler_info;

typedef struct __Namespace
{
    const char* namespace;
    struct __Namespace* next;
} __Namespace;

static void store_c_filepath(const char* c_filepath)
{
    if(transpiler_info.c_filepaths_size >= transpiler_info.c_filepaths_capacity)
    {
        transpiler_info.c_filepaths_capacity = transpiler_info.c_filepaths_capacity == 0 ? 1 : transpiler_info.c_filepaths_capacity * 2;
        transpiler_info.c_filepaths = memory_realloc(transpiler_info.c_filepaths, transpiler_info.c_filepaths_capacity * sizeof(char*), FRX_MEMORY_CATEGORY_STRING);
    }

    transpiler_info.c_filepaths[transpiler_info.c_filepaths_size++] = strdup(c_filepath);
}

static __Namespace* current_namespace = NULL;

static void write_current_namespace(Transpiler* transpiler)
{
    FRX_ASSERT(transpiler != NULL);

    __Namespace* namespace = current_namespace;

    while(namespace != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, "%s_", namespace->namespace);
        namespace = namespace->next;
    }
}

static __Namespace* get_last_namespace(void)
{
    __Namespace* namespace = current_namespace;

    if(namespace == NULL)
        return NULL;

    while(namespace->next != NULL)
        namespace = namespace->next;

    return namespace;
}

static void append_namespace(const char* namespace)
{
    __Namespace* last = get_last_namespace();

    __Namespace* new = memory_alloc(sizeof(__Namespace), FRX_MEMORY_CATEGORY_UNKNOWN);
    
    if(last == NULL)
        current_namespace = new;
    else
        last->next = new;

    new->namespace = namespace;
    new->next = NULL;
}

static void drop_namespace(void)
{
    FRX_ASSERT(current_namespace != NULL);

    if(current_namespace->next == NULL)
    {
        memory_free(current_namespace);

        current_namespace = NULL;

        return;
    }

    __Namespace* namespace = current_namespace;

    while(namespace->next != NULL && namespace->next->next != NULL)
    {
        namespace = namespace->next;
    }

    memory_free(namespace->next);

    namespace->next = NULL;
}

static FRX_NO_DISCARD b8 create_furox_directory_from_source(const char* filepath, char* furox_filepath)
{
    FRX_ASSERT(filepath != NULL);

    FRX_ASSERT(furox_filepath != NULL);

    const char* last_slash = strrchr(filepath, '/');

    if(last_slash == NULL)
        strcpy(furox_filepath, "furox-c");
    else
    {
        usize path_len = last_slash - filepath;

        FRX_ASSERT(path_len < 1000);

        sprintf(furox_filepath, "furox-c/%.*s", (int)path_len, filepath);
    }

    return mkdir(furox_filepath, 0777) == -1 && errno != EEXIST;
}

static FILE* create_c_file(const char* furox_filepath, const char* filepath)
{
    FRX_ASSERT(furox_filepath != NULL);

    FRX_ASSERT(filepath != NULL);

    char c_filepath[strlen(furox_filepath) + strlen("/") + strlen(filepath) + 1];

    sprintf(c_filepath, "%s/%s.c", furox_filepath, filepath);

    FILE* f = fopen(c_filepath, "w");
    if(f == NULL)
        return NULL;

    store_c_filepath(c_filepath);

    if(fprintf(f, "#include \"Furox.h\"\n#include \"%s.h\"\n", filepath) < 0)
    {
        fclose(f);

        return NULL;
    }

    return f;
}

static FILE* create_header_file(const char* furox_filepath, const char* filepath)
{
    FRX_ASSERT(furox_filepath != NULL);

    FRX_ASSERT(filepath != NULL);

    char header_filepath[strlen(furox_filepath) + strlen("/") + strlen(filepath) + 1];

    sprintf(header_filepath, "%s/%s.h", furox_filepath, filepath);

    FILE* f = fopen(header_filepath, "w");
    if(f == NULL)
        return NULL;

    for(usize i = 0; i < strlen(header_filepath); ++i)
        header_filepath[i] = isalnum(header_filepath[i]) ? toupper(header_filepath[i]) : '_';

    if(fprintf(f, "#ifndef %s\n#define %s\n\n", header_filepath, header_filepath) < 0)
    {
        fclose(f);
        
        return NULL;
    }

    return f;
}

static usize indentation_level = 0;

static void push_indentation_level(void)
{
    ++indentation_level;
}

static void drop_indentation_level(void)
{
    FRX_ASSERT(indentation_level > 0);

    --indentation_level;
}

static void write_indentation_level(Transpiler* transpiler)
{
    FRX_ASSERT(transpiler != NULL);

    for(usize i = 0; i < indentation_level; ++i)
    {
        FRX_TRANSPILER_WRITE(transpiler, "    ");
    }
}

FRX_NO_DISCARD b8 ast_transpile_program(Transpiler* transpiler, const ASTProgram* program, const char* src_filepath)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(program != NULL);

    FRX_ASSERT(src_filepath != NULL);

    transpiler->failed = FRX_FALSE;

    char furox_filepath[strlen(src_filepath) + strlen("furox-c/") + 1];

    if(create_furox_directory_from_source(src_filepath, furox_filepath))
        return FRX_TRUE;

    transpiler->header = create_header_file(furox_filepath, src_filepath);
    if(transpiler->header == NULL)
        return FRX_TRUE;

    transpiler->mode = FRX_TRANSPILER_MODE_HEADER;
    for(usize i = 0; i < list_size(&program->top_level_definitions); ++i)
    {
        ast_transpile(transpiler, list_get(&program->top_level_definitions, i));

        FRX_TRANSPILER_WRITE(transpiler, "\n");
    }

    FRX_TRANSPILER_WRITE_HEADER(transpiler, "#endif\n");

    fclose(transpiler->header);

    transpiler->source = create_c_file(furox_filepath, src_filepath);
    if(transpiler->source == NULL)
        return FRX_TRUE;

    transpiler->mode = FRX_TRANSPILER_MODE_SOURCE;
    for(usize i = 0; i < list_size(&program->top_level_definitions); ++i)
    {
        ast_transpile(transpiler, list_get(&program->top_level_definitions, i));

        FRX_TRANSPILER_WRITE(transpiler, "\n");
    }

    fclose(transpiler->source);

    return transpiler->failed;
}

void ast_transpile(Transpiler* transpiler, const AST* ast)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(ast != NULL);

    switch(ast->type)
    {
        case FRX_AST_TYPE_NUMBER: ast_transpile_number(transpiler, ast->node); break;
        case FRX_AST_TYPE_CHAR_LITERAL: ast_transpile_char_literal(transpiler, ast->node); break;
        case FRX_AST_TYPE_STRING_LITERAL: ast_transpile_string_literal(transpiler, ast->node); break;
        case FRX_AST_TYPE_VARIABLE_DECLARATION: ast_transpile_variable_declaration(transpiler, ast->node); break;
        case FRX_AST_TYPE_VARIABLE_DEFINITION: ast_transpile_variable_definition(transpiler, ast->node); break;
        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT: ast_transpile_variable_assignment(transpiler, ast->node); break;
        case FRX_AST_TYPE_VARIABLE: ast_transpile_variable(transpiler, ast->node); break;
        case FRX_AST_TYPE_VARIABLE_ARRAY_ACCESS: ast_transpile_variable_array_access(transpiler, ast->node); break;
        case FRX_AST_TYPE_UNARY_EXPRESSION: ast_transpile_unary_expression(transpiler, ast->node); break;
        case FRX_AST_TYPE_BINARY_EXPRESSION: ast_transpile_binary_expression(transpiler, ast->node); break;
        case FRX_AST_TYPE_IMPORT_STATEMENT: ast_transpile_import_statement(transpiler, ast->node); break;
        case FRX_AST_TYPE_IF_STATEMENT: ast_transpile_if_statement(transpiler, ast->node); break;
        case FRX_AST_TYPE_SWITCH_STATEMENT: ast_transpile_switch_statement(transpiler, ast->node); break;
        case FRX_AST_TYPE_BREAK_STATEMENT: ast_transpile_break_statement(transpiler, ast->node); break;
        case FRX_AST_TYPE_FOR_LOOP: ast_transpile_for_loop(transpiler, ast->node); break;
        case FRX_AST_TYPE_WHILE_LOOP: ast_transpile_while_loop(transpiler, ast->node); break;
        case FRX_AST_TYPE_DO_WHILE_LOOP: ast_transpile_do_while_loop(transpiler, ast->node); break;
        case FRX_AST_TYPE_RETURN_STATEMENT: ast_transpile_return_statement(transpiler, ast->node); break;
        case FRX_AST_TYPE_FUNCTION_DEFINITION: ast_transpile_function_definition(transpiler, ast->node); break;
        case FRX_AST_TYPE_FUNCTION_CALL: ast_transpile_function_call(transpiler, ast->node); break;
        case FRX_AST_TYPE_ENUM_DEFINITION: ast_transpile_enum_definition(transpiler, ast->node); break;
        case FRX_AST_TYPE_STRUCT_DEFINITION: ast_transpile_struct_definition(transpiler, ast->node); break;
        case FRX_AST_TYPE_NAMESPACE: ast_transpile_namespace(transpiler, ast->node); break;
        case FRX_AST_TYPE_EXTERN_BLOCK: ast_transpile_extern_block(transpiler, ast->node); break;
        case FRX_AST_TYPE_MACRO: ast_transpile_macro(transpiler, ast->node); break;
        case FRX_AST_TYPE_SIZEOF: ast_transpile_sizeof(transpiler, ast->node); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void ast_transpile_number(Transpiler* transpiler, const ASTNumber* number)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(number != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "%zu", number->number);
}

void ast_transpile_char_literal(Transpiler* transpiler, const ASTCharLiteral* char_literal)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(char_literal != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "'%s'", char_literal->literal);
}

void ast_transpile_string_literal(Transpiler* transpiler, const ASTStringLiteral* string_literal)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(string_literal != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "\"%s\"", string_literal->literal);
}

void ast_transpile_typename(Transpiler* transpiler, const ASTTypename* type)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(type != NULL);

    if(type->namespace_ref != NULL)
        ast_transpile_namespace_ref(transpiler, type->namespace_ref);

    FRX_TRANSPILER_WRITE(transpiler, "%s", type->name);

    for(usize i = 0; i < type->pointer_level; ++i)
    {
        FRX_TRANSPILER_WRITE(transpiler, "*");
    }
}

void ast_transpile_variable_declaration(Transpiler* transpiler, const ASTVariableDeclaration* variable_declaration)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(variable_declaration != NULL);

    if(transpiler->mode != FRX_TRANSPILER_MODE_SOURCE)
        return;

    ast_transpile_typename(transpiler, variable_declaration->type);

    VariableSymbol* symbol = variable_declaration->variable_symbol;

    FRX_TRANSPILER_WRITE(transpiler, " %s", symbol->name);

    if(variable_declaration->type->array_size != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, "[");
        ast_transpile(transpiler, variable_declaration->type->array_size);
        FRX_TRANSPILER_WRITE(transpiler, "]");
    }

    FRX_TRANSPILER_WRITE(transpiler, ";");
}

void ast_transpile_variable_definition(Transpiler* transpiler, const ASTVariableDefinition* variable_definition)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(variable_definition != NULL);

    if(transpiler->mode != FRX_TRANSPILER_MODE_SOURCE)
        return;

    ast_transpile_typename(transpiler, variable_definition->type);

    VariableSymbol* symbol = variable_definition->variable_symbol;

    FRX_TRANSPILER_WRITE(transpiler, " %s", symbol->name);

    if(variable_definition->type->array_size != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, "[");
        ast_transpile(transpiler, variable_definition->type->array_size);
        FRX_TRANSPILER_WRITE(transpiler, "]");
    }

    FRX_TRANSPILER_WRITE(transpiler, " = ");

    if(variable_definition->value != NULL)
        ast_transpile(transpiler, variable_definition->value);
    else
    {
        FRX_TRANSPILER_WRITE(transpiler, "{\n");

        for(usize i = 0; i < list_size(&variable_definition->array_initialization); ++i)
        {
            AST* array_entry = list_get(&variable_definition->array_initialization, i);

            write_indentation_level(transpiler);
            ast_transpile(transpiler, array_entry);
            FRX_TRANSPILER_WRITE(transpiler, ",\n");
        }

        write_indentation_level(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "}");
    }

    FRX_TRANSPILER_WRITE(transpiler, ";");
}

void ast_transpile_variable_assignment(Transpiler* transpiler, const ASTVariableAssignment* variable_assignment)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(variable_assignment != NULL);

    ast_transpile_variable(transpiler, variable_assignment->variable);

    FRX_TRANSPILER_WRITE(transpiler, " = ");

    ast_transpile(transpiler, variable_assignment->value);
}

void ast_transpile_variable(Transpiler* transpiler, const ASTVariable* variable)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(variable != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "%s", variable->name);

    if(variable->array_index != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, "[");
        ast_transpile(transpiler, variable->array_index);
        FRX_TRANSPILER_WRITE(transpiler, "]");
    }

    if(variable->next != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, variable->is_pointer ? "->" : ".");

        ast_transpile_variable(transpiler, variable->next);
    }
}

void ast_transpile_variable_array_access(Transpiler* transpiler, const ASTVariableArrayAccess* variable_array_access)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(variable_array_access != NULL);

    ast_transpile_variable(transpiler, variable_array_access->variable);

    FRX_TRANSPILER_WRITE(transpiler, "[");
    ast_transpile(transpiler, variable_array_access->index);
    FRX_TRANSPILER_WRITE(transpiler, "] = ");

    ast_transpile(transpiler, variable_array_access->value);
}

void ast_transpile_binary_expression(Transpiler* transpiler, const ASTBinaryExpression* binary_expression)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(binary_expression != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "(");

    ast_transpile(transpiler, binary_expression->left);

    switch(binary_expression->type)
    {
        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT: FRX_TRANSPILER_WRITE(transpiler, " = "); break;

        case FRX_AST_TYPE_ADDITION: FRX_TRANSPILER_WRITE(transpiler, " + "); break;
        case FRX_AST_TYPE_SUBTRACTION: FRX_TRANSPILER_WRITE(transpiler, " - "); break;
        case FRX_AST_TYPE_MULTIPLICATION: FRX_TRANSPILER_WRITE(transpiler, " * "); break;
        case FRX_AST_TYPE_DIVISION: FRX_TRANSPILER_WRITE(transpiler, " / "); break;
        case FRX_AST_TYPE_MODULO: FRX_TRANSPILER_WRITE(transpiler, " %% "); break;
        case FRX_AST_TYPE_LOGICAL_AND: FRX_TRANSPILER_WRITE(transpiler, " && "); break;
        case FRX_AST_TYPE_LOGICAL_OR: FRX_TRANSPILER_WRITE(transpiler, " || "); break;
        case FRX_AST_TYPE_BINARY_AND: FRX_TRANSPILER_WRITE(transpiler, " & "); break;
        case FRX_AST_TYPE_BINARY_OR: FRX_TRANSPILER_WRITE(transpiler, " | "); break;
        case FRX_AST_TYPE_BINARY_XOR: FRX_TRANSPILER_WRITE(transpiler, " ^ "); break;
        case FRX_AST_TYPE_BINARY_LEFT_SHIFT: FRX_TRANSPILER_WRITE(transpiler, " << "); break;
        case FRX_AST_TYPE_BINARY_RIGHT_SHIFT: FRX_TRANSPILER_WRITE(transpiler, " >> "); break;
        case FRX_AST_TYPE_COMPARISON: FRX_TRANSPILER_WRITE(transpiler, " == "); break;
        case FRX_AST_TYPE_GREATER_THAN: FRX_TRANSPILER_WRITE(transpiler, " > "); break;
        case FRX_AST_TYPE_GREATER_THAN_EQUALS: FRX_TRANSPILER_WRITE(transpiler, " >= "); break;
        case FRX_AST_TYPE_LESS_THAN: FRX_TRANSPILER_WRITE(transpiler, " < "); break;
        case FRX_AST_TYPE_LESS_THAN_EQUALS: FRX_TRANSPILER_WRITE(transpiler, " <= "); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    ast_transpile(transpiler, binary_expression->right);

    FRX_TRANSPILER_WRITE(transpiler, ")");
}

void ast_transpile_unary_expression(Transpiler* transpiler, const ASTUnaryExpression* unary_expression)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(unary_expression != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "(");

    switch(unary_expression->type)
    {
        case FRX_AST_TYPE_ARITHMETIC_NEGATION: FRX_TRANSPILER_WRITE(transpiler, "-"); break;
        case FRX_AST_TYPE_LOGICAL_NEGATION: FRX_TRANSPILER_WRITE(transpiler, "!"); break;
        case FRX_AST_TYPE_BINARY_NEGATION: FRX_TRANSPILER_WRITE(transpiler, "~"); break;
        case FRX_AST_TYPE_DEREFERENCE: FRX_TRANSPILER_WRITE(transpiler, "*"); break;
        case FRX_AST_TYPE_ADDRESS_OF: FRX_TRANSPILER_WRITE(transpiler, "&"); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    ast_transpile(transpiler, unary_expression->operand);

    FRX_TRANSPILER_WRITE(transpiler, ")");
}

void ast_transpile_import_statement(Transpiler* transpiler, const ASTImportStatement* import_statement)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(import_statement != NULL);

    if(transpiler->mode != FRX_TRANSPILER_MODE_HEADER)
        return;

    FRX_TRANSPILER_WRITE(transpiler, "#include \"%s.h\"\n", import_statement->filepath);
}

void ast_transpile_if_statement(Transpiler* transpiler, const ASTIfStatement* if_statement)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(if_statement != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "if(");
    ast_transpile(transpiler, if_statement->condition);
    FRX_TRANSPILER_WRITE(transpiler, ")\n");
    ast_transpile_scope(transpiler, if_statement->if_block);

    if(if_statement->else_block != NULL)
    {
        write_indentation_level(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "else\n");
        ast_transpile_scope(transpiler, if_statement->else_block);
    }
}

void ast_transpile_switch_statement(Transpiler *transpiler, const ASTSwitchStatement *switch_statement)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(switch_statement != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "switch(");
    ast_transpile(transpiler, switch_statement->switch_value);
    FRX_TRANSPILER_WRITE(transpiler, ")\n");

    write_indentation_level(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "{\n");

    for(usize i = 0; i < list_size(&switch_statement->cases); ++i)
    {
        ASTSwitchCase* switch_case = list_get(&switch_statement->cases, i);

        write_indentation_level(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "case ");
        ast_transpile(transpiler, switch_case->case_expr);
        FRX_TRANSPILER_WRITE(transpiler, ":\n");

        ast_transpile_scope(transpiler, switch_case->scope);
    }

    if(switch_statement->default_case != NULL)
    {
        write_indentation_level(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "default:\n");
        ast_transpile_scope(transpiler, switch_statement->default_case);
    }

    write_indentation_level(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "}\n");
}

void ast_transpile_break_statement(Transpiler* transpiler, const ASTBreakStatement* break_statement)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(break_statement != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "break");
}

void ast_transpile_for_loop(Transpiler* transpiler, const ASTForLoop* for_loop)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(for_loop != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "for(");
    ast_transpile(transpiler, for_loop->expression);
    FRX_TRANSPILER_WRITE(transpiler, ";");
    ast_transpile(transpiler, for_loop->condition);
    FRX_TRANSPILER_WRITE(transpiler, ";");
    ast_transpile(transpiler, for_loop->increment);
    FRX_TRANSPILER_WRITE(transpiler, ")\n");
    ast_transpile_scope(transpiler, for_loop->scope);
}

void ast_transpile_while_loop(Transpiler* transpiler, const ASTWhileLoop* while_loop)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(while_loop != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "while(");
    ast_transpile(transpiler, while_loop->condition);
    FRX_TRANSPILER_WRITE(transpiler, ")\n");

    ast_transpile_scope(transpiler, while_loop->scope);
}

void ast_transpile_do_while_loop(Transpiler* transpiler, const ASTDoWhileLoop* do_while_loop)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(do_while_loop != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "do\n");
    ast_transpile_scope(transpiler, do_while_loop->scope);
    write_indentation_level(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "while(");
    ast_transpile(transpiler, do_while_loop->condition);
    FRX_TRANSPILER_WRITE(transpiler, " );\n");
}

void ast_transpile_return_statement(Transpiler* transpiler, const ASTReturnStatement* return_statement)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(return_statement != NULL);

    if(return_statement->value != NULL)
    {
        FRX_TRANSPILER_WRITE(transpiler, "return ");
        ast_transpile(transpiler, return_statement->value);
    }
    else
    {
        FRX_TRANSPILER_WRITE(transpiler, "return");
    }
}

void ast_transpile_parameter_list(Transpiler* transpiler, const ASTParameterList* parameter_list)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(parameter_list != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "(");

    usize parameters_count = list_size(&parameter_list->parameters);

    if(parameters_count == 0)
    {
        FRX_TRANSPILER_WRITE(transpiler, "void");
    }

    for(usize i = 0; i < parameters_count; ++i)
    {
        ASTVariableDeclaration* parameter = list_get(&parameter_list->parameters, i);
        ast_transpile_typename(transpiler, parameter->type);
        FRX_TRANSPILER_WRITE(transpiler, " %s", parameter->name);

        if(i != parameters_count - 1)
        {
            FRX_TRANSPILER_WRITE(transpiler, ", ");
        }
    }

    if(parameter_list->is_variadic)
    {
        FRX_TRANSPILER_WRITE(transpiler, ", ...");
    }

    FRX_TRANSPILER_WRITE(transpiler, ")");
}

void ast_transpile_function_definition(Transpiler* transpiler, const ASTFunctionDefinition* function_definition)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(function_definition != NULL);

    FunctionSymbol* symbol = function_definition->function_symbol;

    if(transpiler->mode == FRX_TRANSPILER_MODE_HEADER)
    {
        if(!function_definition->exported)
            return;

        ast_transpile_typename(transpiler, function_definition->type);
        FRX_TRANSPILER_WRITE(transpiler, " ");
        write_current_namespace(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "%s", symbol->name);
        ast_transpile_parameter_list(transpiler, function_definition->parameter_list);
        FRX_TRANSPILER_WRITE(transpiler, ";\n");
    }
    else
    {
        ast_transpile_typename(transpiler, function_definition->type);
        FRX_TRANSPILER_WRITE(transpiler, " ");
        write_current_namespace(transpiler);
        FRX_TRANSPILER_WRITE(transpiler, "%s", symbol->name);
        ast_transpile_parameter_list(transpiler, function_definition->parameter_list);
        FRX_TRANSPILER_WRITE(transpiler, "\n");

        ast_transpile_scope(transpiler, function_definition->scope);
    }
}

void ast_transpile_function_declaration(Transpiler* transpiler, const ASTFunctionDeclaration* function_declaration)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(function_declaration != NULL);

    FunctionSymbol* symbol = function_declaration->function_symbol;

    ast_transpile_typename(transpiler, function_declaration->type);
    FRX_TRANSPILER_WRITE(transpiler, " ");
    write_current_namespace(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "%s", symbol->name);
    ast_transpile_parameter_list(transpiler, function_declaration->parameter_list);
    FRX_TRANSPILER_WRITE(transpiler, ";");
}

void ast_transpile_function_call(Transpiler* transpiler, const ASTFunctionCall* function_call)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(function_call != NULL);

    if(function_call->namespace_ref != NULL)
        ast_transpile_namespace_ref(transpiler, function_call->namespace_ref);

    FunctionSymbol* symbol = function_call->function_symbol;

    FRX_TRANSPILER_WRITE(transpiler, "%s(", symbol->name);

    usize arg_count = list_size(&function_call->arguments);
    for(usize i = 0; i < arg_count; ++i)
    {
        AST* arg = list_get(&function_call->arguments, i);
        ast_transpile(transpiler, arg);

        if(i != arg_count - 1)
        {
            FRX_TRANSPILER_WRITE(transpiler, ", ");
        }
    }

    FRX_TRANSPILER_WRITE(transpiler, ")");
}

void ast_transpile_scope(Transpiler* transpiler, const ASTScope* scope)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(scope != NULL);

    write_indentation_level(transpiler);

    FRX_TRANSPILER_WRITE(transpiler, "{\n");

    push_indentation_level();

    for(usize i = 0; i < list_size(&scope->statements); ++i)
    {
        AST* statement = list_get(&scope->statements, i);

        write_indentation_level(transpiler);

        ast_transpile(transpiler, statement);

        if(statement->type != FRX_AST_TYPE_IF_STATEMENT
                && statement->type != FRX_AST_TYPE_FOR_LOOP
                && statement->type != FRX_AST_TYPE_WHILE_LOOP
                && statement->type != FRX_AST_TYPE_DO_WHILE_LOOP
                && statement->type != FRX_AST_TYPE_VARIABLE_DECLARATION
                && statement->type != FRX_AST_TYPE_VARIABLE_DEFINITION)
        {
            FRX_TRANSPILER_WRITE(transpiler, ";");
        }

        FRX_TRANSPILER_WRITE(transpiler, "\n");
    }

    drop_indentation_level();

    write_indentation_level(transpiler);

    FRX_TRANSPILER_WRITE(transpiler, "}\n");
}

void ast_transpile_enum_definition(Transpiler* transpiler, const ASTEnumDefinition* enum_definition)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(enum_definition != NULL);

    if(transpiler->mode == FRX_TRANSPILER_MODE_HEADER && !enum_definition->exported)
        return;

    if(transpiler->mode == FRX_TRANSPILER_MODE_SOURCE && enum_definition->exported)
        return;

    FRX_TRANSPILER_WRITE(transpiler, "enum\n{\n");

    push_indentation_level();

    usize size = list_size(&enum_definition->constants);
    for(usize i = 0; i < size; ++i)
    {
        AST* constant = list_get(&enum_definition->constants, i);

        write_indentation_level(transpiler);

        ast_transpile(transpiler, constant);

        if(i != size - 1)
            FRX_TRANSPILER_WRITE(transpiler, ",");

        FRX_TRANSPILER_WRITE(transpiler, "\n");
    }

    drop_indentation_level();

    FRX_TRANSPILER_WRITE(transpiler, "};\n\ntypedef ");

    ast_transpile_typename(transpiler, enum_definition->type);

    FRX_TRANSPILER_WRITE(transpiler, " %s;\n", enum_definition->name);
}

void ast_transpile_struct_definition(Transpiler* transpiler, const ASTStructDefinition* struct_definition)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(struct_definition != NULL);

    if((struct_definition->exported && transpiler->mode == FRX_TRANSPILER_MODE_SOURCE)
            || (!struct_definition->exported && transpiler->mode == FRX_TRANSPILER_MODE_HEADER))
        return;

    StructSymbol* symbol = struct_definition->struct_symbol;

    FRX_TRANSPILER_WRITE(transpiler, "typedef struct ");
    write_current_namespace(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "%s\n{\n", symbol->name);

    push_indentation_level();

    for(usize i = 0; i < list_size(&struct_definition->fields); ++i)
    {
        write_indentation_level(transpiler);

        ASTVariableDeclaration* var = list_get(&struct_definition->fields, i);

        ast_transpile_typename(transpiler, var->type);
        FRX_TRANSPILER_WRITE(transpiler, " %s", var->name);

        if(var->type->array_size != NULL)
        {
            FRX_TRANSPILER_WRITE(transpiler, "[");
            ast_transpile(transpiler, var->type->array_size);
            FRX_TRANSPILER_WRITE(transpiler, "]");
        }

        FRX_TRANSPILER_WRITE(transpiler, ";\n");
    }

    drop_indentation_level();

    FRX_TRANSPILER_WRITE(transpiler, "} ");
    write_current_namespace(transpiler);
    FRX_TRANSPILER_WRITE(transpiler, "%s;\n", symbol->name);
}

void ast_transpile_namespace(Transpiler* transpiler, const ASTNamespace* namespace)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(namespace != NULL);

    append_namespace(namespace->name);

    for(usize i = 0; i < list_size(&namespace->top_level_definitions); ++i)
    {
        ast_transpile(transpiler, list_get(&namespace->top_level_definitions, i));

        FRX_TRANSPILER_WRITE(transpiler, "\n");
    }

    drop_namespace();
}

void ast_transpile_namespace_ref(Transpiler* transpiler, const ASTNamespaceRef* namespace_ref)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(namespace_ref != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "%s_", namespace_ref->name);

    if(namespace_ref->next != NULL)
        ast_transpile_namespace_ref(transpiler, namespace_ref->next);
}

void ast_transpile_extern_block(Transpiler* transpiler, const ASTExternBlock* extern_block)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(extern_block != NULL);

    if(transpiler->mode != FRX_TRANSPILER_MODE_SOURCE)
        return;

    for(usize i = 0; i < list_size(&extern_block->struct_definitions); ++i)
    {
        FRX_TRANSPILER_WRITE(transpiler, "extern ");

        ASTStructDefinition* struct_definition = list_get(&extern_block->struct_definitions, i);

        ast_transpile_struct_definition(transpiler, struct_definition);
    }

    for(usize i = 0; i < list_size(&extern_block->function_declarations); ++i)
    {
        FRX_TRANSPILER_WRITE(transpiler, "extern ");

        ASTFunctionDeclaration* function_declaration = list_get(&extern_block->function_declarations, i);

        ast_transpile_function_declaration(transpiler, function_declaration);
    }
}

void ast_transpile_macro(Transpiler* transpiler, const ASTMacro* macro)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(macro != NULL);

    if(!(transpiler->mode == FRX_TRANSPILER_MODE_SOURCE && !macro->exported)
        && !(transpiler->mode == FRX_TRANSPILER_MODE_HEADER && macro->exported))
       return;

    FRX_TRANSPILER_WRITE(transpiler, "#define %s (", macro->name);
    ast_transpile(transpiler, macro->value);
    FRX_TRANSPILER_WRITE(transpiler, ")\n");
}

void ast_transpile_sizeof(Transpiler* transpiler, const ASTSizeof* _sizeof)
{
    FRX_ASSERT(transpiler != NULL);

    FRX_ASSERT(_sizeof != NULL);

    FRX_TRANSPILER_WRITE(transpiler, "sizeof(%s)", _sizeof->type);
}

static FRX_NO_DISCARD b8 generate_furox_main_c()
{
    FILE* f = fopen("furox-c/FuroxMain.c", "w");
    if(f == NULL)
        return FRX_TRUE;

    static const char* furox_main_c = "#include \"Furox.h\"\n\n"
                                      "int Main(void);\n\n"
                                      "int main(int argc, char** argv)\n"
                                      "{\n"
                                      "    return Main();\n"
                                      "}";

    fprintf(f, "%s", furox_main_c);

    fclose(f);

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 generate_furox_header()
{
    FILE* f = fopen("furox-c/Furox.h", "w");
    if(f == NULL)
        return FRX_TRUE;

    static const char* furox_header = "#ifndef FUROX_H\n"
                                      "#define FUROX_H\n\n"
                                      "#include <stddef.h>\n"
                                      "#include <stdint.h>\n\n"
                                      "typedef size_t usize;\n"
                                      "typedef int64_t isize;\n\n"
                                      "typedef int8_t i8;\n"
                                      "typedef int16_t i16;\n"
                                      "typedef int32_t i32;\n"
                                      "typedef int64_t i64;\n\n"
                                      "typedef uint8_t u8;\n"
                                      "typedef uint16_t u16;\n"
                                      "typedef uint32_t u32;\n"
                                      "typedef uint64_t u64;\n\n"
                                      "typedef u8 b8;\n"
                                      "typedef u16 b16;\n"
                                      "typedef u32 b32;\n"
                                      "typedef u64 b64;\n\n"
                                      "typedef float f32;\n"
                                      "typedef double f64;\n\n"
                                      "#endif";

    fprintf(f, "%s", furox_header);

    fclose(f);

    return FRX_FALSE;
}

FRX_NO_DISCARD b8 generate_executable(void)
{
    char command[4096];

    if(generate_furox_main_c())
        return FRX_TRUE;

    if(generate_furox_header())
        return FRX_TRUE;

    strcpy(command, "gcc furox-c/FuroxMain.c");

    for(usize i = 0; i < transpiler_info.c_filepaths_size; ++i)
    {
        strcat(command, " ");
        strcat(command, transpiler_info.c_filepaths[i]);
    }

    return system(command) == -1;
}
