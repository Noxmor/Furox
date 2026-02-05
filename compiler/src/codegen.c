#include "codegen.h"

#include "assert.h"
#include "ast.h"
#include "log.h"
#include "module.h"
#include "symbol_table.h"
#include "temp_dir.h"
#include "source_file.h"
#include "type_system.h"

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
    list_init(&ctx->symbol_list);

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

static void emit_ast(AST* ast, FILE* f);

static void emit_type(const Type* type, FILE* f)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(f != NULL);

    switch (type->kind)
    {
        case FRX_TYPE_KIND_PRIMITIVE: fprintf(f, "%s", token_type_to_str(type->primitive.type)); break;
        case FRX_TYPE_KIND_SYMBOL:
        {
            const Symbol* symbol = type->symbol.symbol;
            switch (symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: fprintf(f, "%s", ((ASTStructDef*)symbol->data)->name); break;
                default: FRX_ASSERT(FRX_FALSE); break;
            }

            fprintf(f, "%p", symbol->data);

            break;
        }
        case FRX_TYPE_KIND_PTR: emit_type(type->ptr.base, f); fprintf(f, "*"); break;
        case FRX_TYPE_KIND_ARRAY: emit_type(type->array.base, f); fprintf(f, "[%zu]", type->array.size); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void emit_enum_definition(ASTEnumDef* enum_def, FILE* f)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(f != NULL);

    fprintf(f, "enum\n{\n");

    for (usize i = 0; i < list_size(&enum_def->constants); ++i)
    {
        AST* ast = list_get(&enum_def->constants, i);
        ASTEnumConstant* constant = &ast->enum_constant;

        fprintf(f, "%s%p", constant->name, constant);

        if (constant->value != NULL)
        {
            fprintf(f, " = ");
            emit_ast(constant->value, f);
        }

        fprintf(f, ",\n");
    }

    fprintf(f, "};\ntypedef ");
    emit_type(enum_def->resolved_type, f);
    fprintf(f, " %s%p;\n", enum_def->name, enum_def);
}

static void emit_struct_declaration(ASTStructDef* struct_def, FILE* f)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(f != NULL);

    fprintf(f, "typedef struct %s%p %s%p;\n", struct_def->name, struct_def, struct_def->name, struct_def);
}

static void emit_struct_field(ASTStructField* struct_field, FILE* f)
{
    FRX_ASSERT(struct_field != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(struct_field->type->type_specifier.resolved_type, f);
    fprintf(f, " %s;\n", struct_field->name);
}

static void emit_struct_definition(ASTStructDef* struct_def, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(f != NULL);

    // Check if we are already transpiled
    if (list_contains(&ctx->symbol_list, struct_def))
    {
        return;
    }

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* struct_field = list_get(&struct_def->fields, i);
        Type* type = struct_field->struct_field.type->type_specifier.resolved_type;

        if (type->kind == FRX_TYPE_KIND_SYMBOL)
        {
            // Found dependency
            const Symbol* symbol = type->symbol.symbol;
            switch (symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: emit_struct_definition(symbol->data, f, ctx);
                case FRX_SYMBOL_TYPE_UNION: break; // TODO: Implement

                default: break;
            }
        }
    }

    fprintf(f, "struct %s%p\n{\n", struct_def->name, struct_def);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* struct_field = list_get(&struct_def->fields, i);
        emit_struct_field(&struct_field->struct_field, f);
    }

    fprintf(f, "};\n");

    // Mark ourself as transpiled
    list_add(&ctx->symbol_list, struct_def);
}

static void emit_func_param(ASTFuncParam* func_param, FILE* f)
{
    FRX_ASSERT(func_param != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(func_param->type->type_specifier.resolved_type, f);
    fprintf(f, " %s%p", func_param->name, func_param);
}

static void emit_func_sig(ASTFuncDecl* func_decl, FILE* f)
{
    FRX_ASSERT(func_decl != NULL);

    FRX_ASSERT(f != NULL);

    emit_type(func_decl->return_type->type_specifier.resolved_type, f);
    fprintf(f, " %s", func_decl->name);

    if (strcmp(func_decl->name, "main") != 0)
    {
        fprintf(f, "%p", func_decl);
    }

    fprintf(f, "(");

    for (usize i = 0; i < list_size(&func_decl->params); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        AST* func_param = list_get(&func_decl->params, i);
        emit_func_param(&func_param->func_param, f);
    }

    fprintf(f, ")");
}

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

    const char* last_path = list_get(&path_expr->path_segments, list_size(&path_expr->path_segments) - 1);
    fprintf(f, "%s", last_path);

    if (list_size(&path_expr->path_segments) > 1 || strcmp(last_path, "main") != 0)
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
    emit_type(let_stmt->resolved_type, f);
    fprintf(f, " %s%p", let_stmt->name, let_stmt);

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

    const char* access_token = expr_infer_type(field_expr->base)->kind == FRX_TYPE_KIND_PTR ? "->" : ".";

    fprintf(f, "%s%s", access_token, field_expr->field_name);
}

static void emit_call_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(f != NULL);

    ASTCallExpr* call_expr = &ast->call_expr;

    emit_ast(call_expr->callee, f);

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

static void emit_method_call_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_METHOD_CALL_EXPR);

    FRX_ASSERT(f != NULL);

    ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;

    fprintf(f, "%s%p(", method_call_expr->name, method_call_expr->symbol->data);

    if (method_call_expr->callee != NULL)
    {
        if (expr_infer_type(method_call_expr->callee)->kind != FRX_TYPE_KIND_PTR)
        {
            fprintf(f, "&");
        }

        emit_ast(method_call_expr->callee, f);
    }

    for (usize i = 0; i < list_size(&method_call_expr->args); ++i)
    {
        if (i > 0 || method_call_expr->callee != NULL)
        {
            fprintf(f, ", ");
        }

        AST* arg = list_get(&method_call_expr->args, i);
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
        case FRX_AST_TYPE_METHOD_CALL_EXPR: emit_method_call_expr(ast, f); break;
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

static void emit_func_body(ASTFuncDecl* func_decl, FILE* f)
{
    FRX_ASSERT(func_decl != NULL);

    emit_scope(func_decl->body, f);
}

void codegen_context_transpile(CodegenContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    // 1. Emit all enums
    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->global_scope->symbols;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_ENUM: emit_enum_definition(symbol->data, ctx->header); break;
                    default: break;
                }

                entry = entry->next;
            }
        }
    }

    // 2. Emit all struct declarations
    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->global_scope->symbols;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_STRUCT: emit_struct_declaration(symbol->data, ctx->header); break;
                    default: break;
                }

                entry = entry->next;
            }
        }
    }

    // 3. Emit all struct definitions dependency based
    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->global_scope->symbols;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_STRUCT: emit_struct_definition(symbol->data, ctx->header, ctx); break;
                    default: break;
                }

                entry = entry->next;
            }
        }
    }

    // 4. Emit all function declarations
    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->global_scope->symbols;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_FUNC: emit_func_sig(symbol->data, ctx->header); fprintf(ctx->header, ";\n"); break;
                    default: break;
                }

                entry = entry->next;
            }
        }
    }

    List* type_infos = type_system_get_type_infos();
    for (usize i = 0; i < list_size(type_infos); ++i)
    {
        TypeInfo* info = list_get(type_infos, i);
        List* methods = &info->methods;
        for (usize j = 0; j < list_size(methods); ++j)
        {
            Symbol* method = list_get(methods, j);
            emit_func_sig(method->data, ctx->header);
            fprintf(ctx->header, ";\n");
        }
    }

    // 5. Emit all function definitions
    for (usize i = 0; i < list_size(ctx->src_files); ++i)
    {
        SourceFile* src_file = list_get(ctx->src_files, i);
        SymbolTable* symbol_table = &src_file->global_scope->symbols;

        for (usize j = 0; j < FRX_SYMBOL_TABLE_CAPACITY; ++j)
        {
            SymbolTableEntry* entry = symbol_table->entries[j];

            while (entry != NULL)
            {
                Symbol* symbol = entry->symbol;

                switch (symbol->type)
                {
                    case FRX_SYMBOL_TYPE_FUNC:
                    {
                        emit_func_sig(symbol->data, ctx->source);
                        fprintf(ctx->source, "\n");
                        emit_func_body(symbol->data, ctx->source);
                        break;
                    }
                    default: break;
                }

                entry = entry->next;
            }
        }
    }

    for (usize i = 0; i < list_size(type_infos); ++i)
    {
        TypeInfo* info = list_get(type_infos, i);
        List* methods = &info->methods;
        for (usize j = 0; j < list_size(methods); ++j)
        {
            Symbol* method = list_get(methods, j);
            emit_func_sig(method->data, ctx->source);
            fprintf(ctx->source, "\n");
            emit_func_body(method->data, ctx->source);
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
