#include "codegen.h"

#include "assert.h"
#include "ast.h"
#include "log.h"
#include "module.h"
#include "symbol_table.h"
#include "temp_dir.h"
#include "source_file.h"
#include "hir.h"

#include <string.h>

b8 codegen_context_init(CodegenContext* ctx, const Module* root_mod,
                        List* src_files, const char* filename)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(root_mod != NULL);

    FRX_ASSERT(src_files != NULL);

    FRX_ASSERT(filename != NULL);

    FRX_LOG_INFO("Initializing codegen context...");

    ctx->root_mod = root_mod;
    ctx->src_files = src_files;

    const char* temp_dir = temp_dir_path();
    char header_buffer[strlen(temp_dir) + 1 + strlen(filename) + 3];
    char source_buffer[strlen(temp_dir) + 1 + strlen(filename) + 3];
    sprintf(header_buffer, "%s/%s.h", temp_dir, filename);
    sprintf(source_buffer, "%s/%s.c", temp_dir, filename);

    ctx->header = fopen(header_buffer, "w");
    if (ctx->header == NULL)
    {
        return FRX_TRUE;
    }

    ctx->source = fopen(source_buffer, "w");
    if (ctx->source == NULL)
    {
        fclose(ctx->header);
        return FRX_TRUE;
    }

    fprintf(ctx->header, "#ifndef FRX_H\n");
    fprintf(ctx->header, "#define FRX_H\n");

    fprintf(ctx->header, "#include <stddef.h>\n");
    fprintf(ctx->header, "#include <stdint.h>\n");

    fprintf(ctx->header, "typedef int8_t i8;\n");
    fprintf(ctx->header, "typedef int16_t i16;\n");
    fprintf(ctx->header, "typedef int32_t i32;\n");
    fprintf(ctx->header, "typedef int64_t i64;\n");
    fprintf(ctx->header, "typedef int64_t isize;\n");

    fprintf(ctx->header, "typedef uint8_t u8;\n");
    fprintf(ctx->header, "typedef uint16_t u16;\n");
    fprintf(ctx->header, "typedef uint32_t u32;\n");
    fprintf(ctx->header, "typedef uint64_t u64;\n");
    fprintf(ctx->header, "typedef uint64_t usize;\n");

    fprintf(ctx->header, "typedef u8 b8;\n");
    fprintf(ctx->header, "typedef u16 b16;\n");
    fprintf(ctx->header, "typedef u32 b32;\n");
    fprintf(ctx->header, "typedef u64 b64;\n");

    fprintf(ctx->header, "typedef float f32;\n");
    fprintf(ctx->header, "typedef double f64;\n");

    fprintf(ctx->source, "#include \"%s.h\"\n", filename);


    return FRX_FALSE;
}

static void emit_type(const Type* type, FILE* f)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(f != NULL);

    switch (type->kind)
    {
        case FRX_TYPE_KIND_PRIMITIVE: fprintf(f, "%s", token_type_to_str(type->primitive_type)); break;
        case FRX_TYPE_KIND_SYMBOL:
        {
            const Symbol* symbol = type->symbol;
            switch (symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: fprintf(f, "%s", ((StructDef*)symbol->data)->name); break;
                default: FRX_ASSERT(FRX_FALSE); break;
            }

            fprintf(f, "%p", symbol->data);

            break;
        }
        case FRX_TYPE_KIND_PTR: emit_type(type->base_type, f); fprintf(f, "*"); break;
        case FRX_TYPE_KIND_ARRAY: emit_type(type->base_type, f); fprintf(f, "[%zu]", type->size); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void emit_struct_declaration(StructDef* struct_def, FILE* f)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(f != NULL);

    fprintf(f, "typedef struct %s%p %s%p;\n", struct_def->name, struct_def, struct_def->name, struct_def);
}

static void emit_struct_field(StructField* struct_field, FILE* f)
{
    FRX_ASSERT(struct_field != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(struct_field->type, f);
    fprintf(f, " %s;\n", struct_field->name);
}

static void emit_struct_definition(StructDef* struct_def, FILE* f)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(f != NULL);

    fprintf(f, "struct %s%p\n{\n", struct_def->name, struct_def);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* struct_field = list_get(&struct_def->fields, i);
        emit_struct_field(struct_field, f);
    }

    fprintf(f, "};\n");
}

static void emit_func_param(FuncParam* func_param, FILE* f)
{
    FRX_ASSERT(func_param != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(func_param->type, f);
    fprintf(f, " %s%p", func_param->name, func_param);
}

static void emit_func_params(FuncParams* func_params, FILE* f)
{
    FRX_ASSERT(func_params != NULL);

    FRX_ASSERT(f != NULL);

    fprintf(f, "(");

    for (usize i = 0; i < list_size(&func_params->params); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        FuncParam* func_param = list_get(&func_params->params, i);
        emit_func_param(func_param, f);
    }

    fprintf(f, ")");
}

static void emit_func_def_signature(FuncDef* func_def, FILE* f)
{
    FRX_ASSERT(func_def != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(func_def->return_type, f);
    fprintf(f, " %s", func_def->name);

    if (strcmp(func_def->name, "main") != 0)
    {
        fprintf(f, "%p", func_def);
    }

    emit_func_params(func_def->params, f);
}

static void emit_ast(AST* ast, FILE* f);

static void emit_int_literal(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_INT_LIT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "%zu", ast->int_literal.value);
}

static void emit_path_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(f != NULL);

    ASTPathExpr* path_expr = &ast->path_expr;

    const char* last_name = NULL;
    for (usize i = 0; i < list_size(&path_expr->path_segments); ++i)
    {
        if (i > 0)
        {
            fprintf(f, "_");
        }

        const char* name = list_get(&path_expr->path_segments, i);
        last_name = name;
        fprintf(f, "%s", name);
    }

    if (list_size(&path_expr->path_segments) > 1 || strcmp(last_name, "main") != 0)
    {
        FRX_ASSERT(path_expr->symbol != NULL);
        fprintf(f, "%p", path_expr->symbol->data);
    }
}

static void emit_let_stmt(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    FRX_ASSERT(f != NULL);

    ASTLetStmt* let_stmt = &ast->let_stmt;
    Symbol* symbol = let_stmt->symbol;
    Variable* variable = symbol->data;
    emit_type(variable->type, f);
    fprintf(f, " %s%p", let_stmt->name, symbol->data);

    if (let_stmt->value)
    {
        fprintf(f, " = ");
        emit_ast(let_stmt->value, f);
    }

    fprintf(f, ";\n");
}

static void emit_unary_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(f != NULL);

    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    fprintf(f, "(%s", token_type_to_str(unary_expr->type));
    emit_ast(unary_expr->operand, f);
    fprintf(f, ")");
}

static void emit_binary_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    FRX_ASSERT(f != NULL);

    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    fprintf(f, "(");
    emit_ast(binary_expr->left, f);

    if (binary_expr->operator != FRX_OPERATOR_CALL)
    {
        fprintf(f, "%s", token_type_to_str(binary_expr->type));
    }

    emit_ast(binary_expr->right, f);
    fprintf(f, ")");
}

static void emit_field_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(f != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    emit_ast(field_expr->base, f);

    fprintf(f, ".%s", field_expr->field_name);
}

static void emit_call_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(f != NULL);

    ASTCallExpr* call_expr = &ast->call_expr;

    fprintf(f, "(");

    for (usize i = 0; i < list_size(&call_expr->args); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        AST* arg = list_get(&call_expr->args, i);
        emit_ast(arg, f);
    }

    fprintf(f, ")");
}

static void emit_expr_stmt(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(f != NULL);

    emit_ast(ast->expr_stmt.expr, f);
    fprintf(f, ";\n");
}

static void emit_return_stmt(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_RETURN_STMT);

    FRX_ASSERT(f != NULL);

    ASTReturnStmt* return_stmt = &ast->return_stmt;
    if (return_stmt->value)
    {
        fprintf(f, "return ");
        emit_ast(return_stmt->value, f);
        fprintf(f, ";\n");
    }
    else
    {
        fprintf(f, "return;\n");
    }
}

static void emit_ast(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_INT_LIT: emit_int_literal(ast, f); break;
        case FRX_AST_TYPE_PATH_EXPR: emit_path_expr(ast, f); break;
        case FRX_AST_TYPE_LET_STMT: emit_let_stmt(ast, f); break;
        case FRX_AST_TYPE_UNARY_EXPR: emit_unary_expr(ast, f); break;
        case FRX_AST_TYPE_BINARY_EXPR: emit_binary_expr(ast, f); break;
        case FRX_AST_TYPE_FIELD_EXPR: emit_field_expr(ast, f); break;
        case FRX_AST_TYPE_CALL_EXPR: emit_call_expr(ast, f); break;
        case FRX_AST_TYPE_EXPR_STMT: emit_expr_stmt(ast, f); break;
        case FRX_AST_TYPE_RETURN_STMT: emit_return_stmt(ast, f); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void emit_scope(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SCOPE);

    FRX_ASSERT(f != NULL);

    ASTScope* scope = &ast->scope;

    fprintf(f, "{\n");

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        AST* stmt = list_get(&scope->stmts, i);
        emit_ast(stmt, f);
    }

    fprintf(f, "}\n");
}

static void emit_func_def_body(FuncDef* func_def, FILE* f)
{
    FRX_ASSERT(func_def != NULL);

    emit_scope(func_def->body, f);
}

void codegen_context_emit_declarations(CodegenContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->symbol_table;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = &entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_STRUCT: emit_struct_declaration(symbol->data, ctx->header); break;
                    case FRX_SYMBOL_TYPE_FUNC: emit_func_def_signature(symbol->data, ctx->header); fprintf(ctx->header, ";\n"); break;
                    case FRX_SYMBOL_TYPE_PARAM: break;
                    case FRX_SYMBOL_TYPE_VAR: break;
                    default: FRX_ASSERT(FRX_FALSE); break;
                }

                entry = entry->next;
            }
        }
    }
}

void codegen_context_emit_definitions(CodegenContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->symbol_table;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = &entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_STRUCT: emit_struct_definition(symbol->data, ctx->source); break;
                    case FRX_SYMBOL_TYPE_FUNC:
                    {
                        emit_func_def_signature(symbol->data, ctx->source);
                        fprintf(ctx->source, "\n");
                        emit_func_def_body(symbol->data, ctx->source);
                        break;
                    }
                    case FRX_SYMBOL_TYPE_PARAM: break;
                    case FRX_SYMBOL_TYPE_VAR: break;
                    default: FRX_ASSERT(FRX_FALSE); break;
                }

                entry = entry->next;
            }
        }
    }
}

void codegen_context_end(CodegenContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    fprintf(ctx->header, "#endif");

    fflush(ctx->header);
    fflush(ctx->source);

    fclose(ctx->source);
    fclose(ctx->header);
}

void codegen_mangle_module(FILE* f, const Module* mod)
{
    FRX_ASSERT(f != NULL);

    FRX_ASSERT(mod != NULL);

    if (mod->parent != NULL)
    {
        codegen_mangle_module(f, mod->parent);
    }

    fprintf(f, "%s_", mod->name);
}
