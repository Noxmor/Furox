#include "codegen.h"

#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "compiler.h"
#include "log.h"
#include "module.h"
#include "operator.h"
#include "symbol_table.h"
#include "temp_dir.h"
#include "source_file.h"
#include "type_system.h"

#include <string.h>

static void emit_block(AST* ast, FILE* f, CodegenContext* ctx);

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
    ctx->generic_params = NULL;
    ctx->generic_args = NULL;

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

    fprintf(ctx->header, "#define FRX_TRUE 1\n");
    fprintf(ctx->header, "#define FRX_FALSE 0\n");

    fprintf(ctx->header, "typedef float f32;\n");
    fprintf(ctx->header, "typedef double f64;\n");

    fprintf(ctx->source, "#include \"%s.h\"\n", filename);

    return FRX_FALSE;
}

static void emit_ast(AST* ast, FILE* f, CodegenContext* ctx);

static void emit_type(const Type* type, const char* name, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(f != NULL);

    switch (type->kind)
    {
        case FRX_TYPE_KIND_PRIMITIVE: fprintf(f, "%s", token_type_to_str(type->primitive.type)); break;
        case FRX_TYPE_KIND_STRUCT:
        case FRX_TYPE_KIND_UNION: fprintf(f, "%s%p%p", type->strct.symbol->name, type->strct.symbol->data, type); break;
        case FRX_TYPE_KIND_ENUM: fprintf(f, "%s%p", type->enumeration.symbol->name, type->enumeration.symbol->data); break;
        case FRX_TYPE_KIND_FUNC:
        {
            emit_type(type->func.return_type, NULL, f, ctx);
            fprintf(f, " (*%s)(", name);

            for (usize i = 0; i < list_size(&type->func.params); ++i)
            {
                if (i > 0)
                {
                    fprintf(f, ", ");
                }

                const Type* param = list_get(&type->func.params, i);
                emit_type(param, NULL, f, ctx);
            }

            if (type->func.is_variadic)
            {
                fprintf(f, ", ...");
            }

            fprintf(f, ")");

            break;
        }
        case FRX_TYPE_KIND_PTR: emit_type(type->ptr.base, NULL, f, ctx); fprintf(f, "*"); break;
        case FRX_TYPE_KIND_ARRAY:
        {
            emit_type(type->array.base, NULL, f, ctx);
            fprintf(f, " %s[", name);
            emit_ast(type->array.size, f, ctx);
            fprintf(f, "]");
            break;
        }
        case FRX_TYPE_KIND_GENERIC:
        {
            const ASTGenericParams* generic_params = &ctx->generic_params->generic_params;

            FRX_ASSERT(list_size(&generic_params->params) == list_size(ctx->generic_args));
            for (usize i = 0; i < list_size(ctx->generic_args); ++i)
            {
                const AST* generic_param = list_get(&generic_params->params, i);
                const AST* generic_arg = list_get(ctx->generic_args, i);
                if (type->generic.symbol->name == generic_param->generic_param.name)
                {
                    const Type* generic_arg_type = attributes_table_lookup_type(generic_arg->id);
                    emit_type(generic_arg_type, type->generic.symbol->name, f, ctx);
                }
            }

            break;
        }

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void emit_enum_definition(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    ASTEnumDef* enum_def = &ast->enum_def;

    fprintf(f, "enum\n{\n");

    for (usize i = 0; i < list_size(&enum_def->variants); ++i)
    {
        AST* variant_node = list_get(&enum_def->variants, i);
        ASTEnumVariant* variant = &variant_node->enum_variant;

        fprintf(f, "%s%p", variant->name, variant_node);

        if (variant->value != NULL)
        {
            fprintf(f, " = ");
            emit_ast(variant->value, f, ctx);
        }

        fprintf(f, ",\n");
    }

    fprintf(f, "};\ntypedef ");
    const Type* enum_def_type = attributes_table_lookup_type(enum_def->type->id);
    emit_type(enum_def_type, NULL, f, ctx);
    fprintf(f, " %s%p;\n", enum_def->name, ast);
}

static void emit_struct_declaration(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    ASTStructDef* struct_def = &ast->struct_def;

    for (usize i = 0; i < list_size(&struct_def->instantiated_types); ++i)
    {
        const Type* type = list_get(&struct_def->instantiated_types, i);
        fprintf(f, "typedef struct %s%p%p %s%p%p;\n", struct_def->name, ast, type, struct_def->name, ast, type);
    }
}

static void emit_struct_field(ASTStructField* struct_field, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(struct_field != NULL);

    FRX_ASSERT(f != NULL);

    const Type* type = attributes_table_lookup_type(struct_field->type->id);

    emit_type(type, struct_field->name, f, ctx);

    if (type->kind != FRX_TYPE_KIND_FUNC)
    {
        fprintf(f, " %s", struct_field->name);
    }

    fprintf(f, ";\n");
}

static void emit_struct_definition(const Type* type, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(type->kind == FRX_TYPE_KIND_STRUCT || type->kind == FRX_TYPE_KIND_UNION);

    FRX_ASSERT(f != NULL);

    // Check if we are already transpiled
    if (list_contains(&ctx->symbol_list, type))
    {
        return;
    }

    ASTStructDef* struct_def = &type->strct.symbol->data->struct_def;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        const Type* field_type = attributes_table_lookup_type(field->struct_field.type->id);

        if (field_type->kind == FRX_TYPE_KIND_STRUCT || field_type->kind == FRX_TYPE_KIND_UNION)
        {
            // Found dependency
            emit_struct_definition(field_type, f, ctx);
        }
        else if (field_type->kind == FRX_TYPE_KIND_GENERIC)
        {

        }
    }

    fprintf(f, "struct %s%p%p\n{\n", struct_def->name, type->strct.symbol->data, type);

    const AST* prev_generic_params = ctx->generic_params;
    const List* prev_generic_args = ctx->generic_args;

    ctx->generic_params = struct_def->generic_params;
    ctx->generic_args = type->strct.generic_args;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* struct_field = list_get(&struct_def->fields, i);
        emit_struct_field(&struct_field->struct_field, f, ctx);
    }

    ctx->generic_params = prev_generic_params;
    ctx->generic_args = prev_generic_args;

    fprintf(f, "};\n");

    // Mark ourself as transpiled
    list_add(&ctx->symbol_list, (Type*)type);
}

static void emit_func_param(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    ASTFuncParam* func_param = &ast->func_param;

    char mangled_name[strlen(func_param->name) + 2 + 16 + 1];
    sprintf(mangled_name, "%s%p", func_param->name, ast);
    const Type* func_param_type = attributes_table_lookup_type(func_param->type->id);
    emit_type(func_param_type, mangled_name, f, ctx);
    fprintf(f, " %s%p", func_param->name, ast);
}

static void emit_func_sig(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    if (func_decl->external)
    {
        fprintf(f, "extern ");
    }

    const AST* prev_generic_params = ctx->generic_params;
    const List* prev_generic_args = ctx->generic_args;

    ctx->generic_params = func_decl->generic_params;
    ctx->generic_args = NULL;

    char mangled_name[strlen(func_decl->name) + 2 + 16 + 1];
    sprintf(mangled_name, "%s%p", func_decl->name, ast);
    const Type* return_type = attributes_table_lookup_type(func_decl->return_type->id);
    emit_type(return_type, mangled_name, f, ctx);
    fprintf(f, " %s", func_decl->name);

    if (strcmp(func_decl->name, "main") != 0 && !func_decl->external)
    {
        fprintf(f, "%p", ast);
    }

    fprintf(f, "(");

    if (func_decl->receiver != FRX_FUNC_RECEIVER_NONE)
    {
        const Type* receiver_type = attributes_table_lookup_func_receiver_type(ast->id);
        emit_type(receiver_type, NULL, f, ctx);
        fprintf(f, " self");

        if (!list_empty(&func_decl->params))
        {
            fprintf(f, ", ");
        }
    }

    for (usize i = 0; i < list_size(&func_decl->params); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        AST* func_param = list_get(&func_decl->params, i);
        emit_func_param(func_param, f, ctx);
    }

    if (func_decl->is_variadic)
    {
        fprintf(f, ", ...");
    }

    fprintf(f, ")");

    ctx->generic_params = prev_generic_params;
    ctx->generic_args = prev_generic_args;
}

static void emit_int_literal(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_INT_LIT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "%zu", ast->int_literal.value);
}

static void emit_char_literal(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CHAR_LIT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "'%s'", ast->char_literal.value);
}

static void emit_string_literal(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRING_LIT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "\"%s\"", ast->string_literal.value);
}

static void emit_path(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH);

    FRX_ASSERT(f != NULL);

    ASTPath* path = &ast->path;

    AST* path_segment = list_get(&path->path_segments, list_size(&path->path_segments) - 1);
    const char* last_path = path_segment->path_segment.name;
    fprintf(f, "%s", last_path);

    if ((list_size(&path->path_segments) > 1 || strcmp(last_path, "main") != 0)
        && (path->symbol->type != FRX_SYMBOL_TYPE_FUNC || (path->symbol->type == FRX_SYMBOL_TYPE_FUNC && !path->symbol->data->func_decl.external)))
    {
        FRX_ASSERT(path->symbol != NULL);

        fprintf(f, "%p", path->symbol->data);
    }
}

static void emit_path_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(f != NULL);

    ASTPathExpr* path_expr = &ast->path_expr;

    emit_path(path_expr->path, f);
}

static void emit_struct_literal(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT);

    FRX_ASSERT(f != NULL);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteral* literal = &ast->struct_literal;

    fprintf(f, "(");
    const Type* type = symbol_infer_type(literal->path->path.symbol);
    emit_type(type, NULL, f, ctx);
    fprintf(f, ") { ");

    for (usize i = 0; i < list_size(&literal->fields); ++i)
    {
        AST* struct_field = list_get(&literal->fields, i);
        ASTStructLiteralField* field = &struct_field->struct_literal_field;

        fprintf(f, ".%s = ", field->name);
        emit_ast(field->value, f, ctx);
        fprintf(f, ", ");
    }

    fprintf(f, "}");
}

static void emit_let_stmt(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    FRX_ASSERT(f != NULL);

    ASTLetStmt* let_stmt = &ast->let_stmt;
    const Type* type = attributes_table_lookup_type(ast->id);

    char mangled_name[strlen(let_stmt->name) + 2 + 16 + 1];
    sprintf(mangled_name, "%s%p", let_stmt->name, ast);
    emit_type(type, mangled_name, f, ctx);

    if (type->kind != FRX_TYPE_KIND_FUNC && type->kind != FRX_TYPE_KIND_ARRAY)
    {
        fprintf(f, " %s%p", let_stmt->name, ast);
    }

    if (let_stmt->value)
    {
        fprintf(f, " = ");
        emit_ast(let_stmt->value, f, ctx);
    }

    fprintf(f, ";\n");
}

static void emit_if_stmt(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    FRX_ASSERT(f != NULL);

    ASTIfStmt* if_stmt = &ast->if_stmt;

    fprintf(f, "if (");
    emit_ast(if_stmt->condition, f, ctx);
    fprintf(f, ")\n");

    emit_block(if_stmt->then_block, f, ctx);

    if (if_stmt->else_block != NULL)
    {
        fprintf(f, "else\n");
        emit_block(if_stmt->else_block, f, ctx);
    }
}

static void emit_for_loop(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FOR_LOOP);

    FRX_ASSERT(f != NULL);

    ASTForLoop* for_loop = &ast->for_loop;

    fprintf(f, "for (");

    if (for_loop->init != NULL)
    {
        emit_ast(for_loop->init, f, ctx);

        if (for_loop->init->type != FRX_AST_TYPE_LET_STMT)
        {
            fprintf(f, ";");
        }
    }
    else
    {
        fprintf(f, "; ");
    }

    emit_ast(for_loop->condition, f, ctx);
    fprintf(f, "; ");

    if (for_loop->increment != NULL)
    {
        emit_ast(for_loop->increment, f, ctx);
    }

    fprintf(f, ")\n");

    emit_block(for_loop->body, f, ctx);
}

static void emit_while_loop(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_WHILE_LOOP);

    FRX_ASSERT(f != NULL);

    ASTWhileLoop* while_loop = &ast->while_loop;

    fprintf(f, "while (");
    emit_ast(while_loop->condition, f, ctx);
    fprintf(f, ")\n");
    emit_block(while_loop->body, f, ctx);
}

static void emit_loop(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LOOP);

    FRX_ASSERT(f != NULL);

    ASTLoop* loop = &ast->loop;

    fprintf(f, "while (FRX_TRUE)\n");
    emit_block(loop->body, f, ctx);
}

static void emit_unary_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(f != NULL);

    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    if (unary_expr->operator != FRX_OPERATOR_POST_INC
        && unary_expr->operator != FRX_OPERATOR_POST_DEC)
    {
        fprintf(f, "(%s", token_type_to_str(unary_expr->type));
        emit_ast(unary_expr->operand, f, ctx);
        fprintf(f, ")");
    }
    else
    {
        fprintf(f, "(");
        emit_ast(unary_expr->operand, f, ctx);
        fprintf(f, "%s)", token_type_to_str(unary_expr->type));

    }
}

static void emit_binary_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    FRX_ASSERT(f != NULL);

    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    fprintf(f, "(");
    emit_ast(binary_expr->left, f, ctx);

    if (binary_expr->operator != FRX_OPERATOR_CALL)
    {
        fprintf(f, "%s", token_type_to_str(binary_expr->type));
    }

    emit_ast(binary_expr->right, f, ctx);

    if (binary_expr->operator == FRX_OPERATOR_ARRAY_SUBSCRIPT)
    {
        fprintf(f, "]");
    }

    fprintf(f, ")");
}

static void emit_field_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(f != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    emit_ast(field_expr->base, f, ctx);

    const char* access_token = expr_infer_type(field_expr->base)->kind == FRX_TYPE_KIND_PTR ? "->" : ".";

    fprintf(f, "%s%s", access_token, field_expr->field_name);
}

static void emit_self_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SELF_EXPR);

    FRX_ASSERT(f != NULL);

    fprintf(f, "self");
}

static void emit_bool_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BOOL_EXPR);

    FRX_ASSERT(f != NULL);

    ASTBoolExpr* bool_expr = &ast->bool_expr;

    if (bool_expr->value == FRX_TRUE)
    {
        fprintf(f, "FRX_TRUE");
    }
    else
    {
        fprintf(f, "FRX_FALSE");
    }
}

static void emit_nullptr_expr(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_NULLPTR_EXPR);

    FRX_ASSERT(f != NULL);

    fprintf(f, "NULL");
}

static void emit_call_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(f != NULL);

    ASTCallExpr* call_expr = &ast->call_expr;

    emit_ast(call_expr->callee, f, ctx);

    fprintf(f, "(");

    for (usize i = 0; i < list_size(&call_expr->args); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        AST* arg = list_get(&call_expr->args, i);
        emit_ast(arg, f, ctx);
    }

    fprintf(f, ")");
}

static void emit_method_call_expr(AST* ast, FILE* f, CodegenContext* ctx)
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

        emit_ast(method_call_expr->callee, f, ctx);
    }

    for (usize i = 0; i < list_size(&method_call_expr->args); ++i)
    {
        if (i > 0 || method_call_expr->callee != NULL)
        {
            fprintf(f, ", ");
        }

        AST* arg = list_get(&method_call_expr->args, i);
        emit_ast(arg, f, ctx);
    }

    fprintf(f, ")");
}

static void emit_cast_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CAST_EXPR);

    FRX_ASSERT(f != NULL);

    ASTCastExpr* cast_expr = &ast->cast_expr;

    fprintf(f, "((");
    const Type* type = attributes_table_lookup_type(cast_expr->type_specifier->id);
    emit_type(type, NULL, f, ctx);
    fprintf(f, ") ");
    emit_ast(cast_expr->expr, f, ctx);
    fprintf(f, ")");
}

static void emit_sizeof_expr(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SIZEOF_EXPR);

    FRX_ASSERT(f != NULL);

    ASTSizeofExpr* sizeof_expr = &ast->sizeof_expr;

    fprintf(f, "sizeof(");
    emit_ast(sizeof_expr->expr, f, ctx);
    fprintf(f, ")");
}

static void emit_expr_stmt(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(f != NULL);

    emit_ast(ast->expr_stmt.expr, f, ctx);
    fprintf(f, ";\n");
}

static void emit_break_stmt(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BREAK_STMT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "break;\n");
}

static void emit_continue_stmt(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CONTINUE_STMT);

    FRX_ASSERT(f != NULL);

    fprintf(f, "continue;\n");
}

static void emit_return_stmt(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_RETURN_STMT);

    FRX_ASSERT(f != NULL);

    ASTReturnStmt* return_stmt = &ast->return_stmt;
    if (return_stmt->value)
    {
        fprintf(f, "return ");
        emit_ast(return_stmt->value, f, ctx);
        fprintf(f, ";\n");
    }
    else
    {
        fprintf(f, "return;\n");
    }
}

static void emit_ast(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(f != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_INT_LIT: emit_int_literal(ast, f); break;
        case FRX_AST_TYPE_CHAR_LIT: emit_char_literal(ast, f); break;
        case FRX_AST_TYPE_STRING_LIT: emit_string_literal(ast, f); break;
        case FRX_AST_TYPE_STRUCT_LIT: emit_struct_literal(ast, f, ctx); break;
        case FRX_AST_TYPE_PATH_EXPR: emit_path_expr(ast, f); break;
        case FRX_AST_TYPE_LET_STMT: emit_let_stmt(ast, f, ctx); break;
        case FRX_AST_TYPE_IF_STMT: emit_if_stmt(ast, f, ctx); break;
        case FRX_AST_TYPE_FOR_LOOP: emit_for_loop(ast, f, ctx); break;
        case FRX_AST_TYPE_WHILE_LOOP: emit_while_loop(ast, f, ctx); break;
        case FRX_AST_TYPE_LOOP: emit_loop(ast, f, ctx); break;
        case FRX_AST_TYPE_UNARY_EXPR: emit_unary_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_BINARY_EXPR: emit_binary_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_FIELD_EXPR: emit_field_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_SELF_EXPR: emit_self_expr(ast, f); break;
        case FRX_AST_TYPE_BOOL_EXPR: emit_bool_expr(ast, f); break;
        case FRX_AST_TYPE_NULLPTR_EXPR: emit_nullptr_expr(ast, f); break;
        case FRX_AST_TYPE_CALL_EXPR: emit_call_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_METHOD_CALL_EXPR: emit_method_call_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_CAST_EXPR: emit_cast_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_SIZEOF_EXPR: emit_sizeof_expr(ast, f, ctx); break;
        case FRX_AST_TYPE_EXPR_STMT: emit_expr_stmt(ast, f, ctx); break;
        case FRX_AST_TYPE_BREAK_STMT: emit_break_stmt(ast, f); break;
        case FRX_AST_TYPE_CONTINUE_STMT: emit_continue_stmt(ast, f); break;
        case FRX_AST_TYPE_RETURN_STMT: emit_return_stmt(ast, f, ctx); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void emit_block(AST* ast, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BLOCK);

    FRX_ASSERT(f != NULL);

    ASTBlock* block = &ast->block;

    fprintf(f, "{\n");

    for (usize i = 0; i < list_size(&block->stmts); ++i)
    {
        AST* stmt = list_get(&block->stmts, i);
        emit_ast(stmt, f, ctx);
    }

    fprintf(f, "}\n");
}

static void emit_func_body(ASTFuncDecl* func_decl, FILE* f, CodegenContext* ctx)
{
    FRX_ASSERT(func_decl != NULL);

    emit_block(func_decl->body, f, ctx);
}

void codegen_context_transpile(CodegenContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    List symbols;
    list_init(&symbols);

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

                if (!list_contains(&symbols, symbol))
                {
                    list_add(&symbols, symbol);
                }

                entry = entry->next;
            }
        }
    }

    // 1. Emit all enums
    for (usize i = 0; i < list_size(&symbols); ++i)
    {
        Symbol* symbol = list_get(&symbols, i);

        switch (symbol->type)
        {
            case FRX_SYMBOL_TYPE_ENUM: emit_enum_definition(symbol->data, ctx->header, ctx); break;
            default: break;
        }
    }

    // 2. Emit all struct declarations
    for (usize i = 0; i < list_size(compiler_get_types()); ++i)
    {
        const Type* type = list_get(compiler_get_types(), i);

        switch (type->kind)
        {
            case FRX_TYPE_KIND_STRUCT:
            case FRX_TYPE_KIND_UNION: emit_struct_declaration(type->strct.symbol->data, ctx->header); break;
            default: break;
        }
    }

    // 3. Emit all struct definitions dependency based
    for (usize i = 0; i < list_size(compiler_get_types()); ++i)
    {
        const Type* type = list_get(compiler_get_types(), i);

        switch (type->kind)
        {
            case FRX_TYPE_KIND_STRUCT:
            case FRX_TYPE_KIND_UNION: emit_struct_definition(type, ctx->header, ctx); break;
            default: break;
        }
    }

    // 4. Emit all function declarations
    for (usize i = 0; i < list_size(&symbols); ++i)
    {
        Symbol* symbol = list_get(&symbols, i);

        switch (symbol->type)
        {
            case FRX_SYMBOL_TYPE_FUNC: emit_func_sig(symbol->data, ctx->header, ctx); fprintf(ctx->header, ";\n"); break;
            default: break;
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
            emit_func_sig(method->data, ctx->header, ctx);
            fprintf(ctx->header, ";\n");
        }
    }

    // 5. Emit all function definitions
    for (usize i = 0; i < list_size(&symbols); ++i)
    {
        Symbol* symbol = list_get(&symbols, i);

        switch (symbol->type)
        {
            case FRX_SYMBOL_TYPE_FUNC:
            {
                if (!symbol->data->func_decl.external)
                {
                    emit_func_sig(symbol->data, ctx->source, ctx);
                    fprintf(ctx->source, "\n");
                    emit_func_body(&symbol->data->func_decl, ctx->source, ctx);
                }

                break;
            }
            default: break;
        }
    }

    for (usize i = 0; i < list_size(type_infos); ++i)
    {
        TypeInfo* info = list_get(type_infos, i);
        List* methods = &info->methods;
        for (usize j = 0; j < list_size(methods); ++j)
        {
            Symbol* method = list_get(methods, j);
            emit_func_sig(method->data, ctx->source, ctx);
            fprintf(ctx->source, "\n");
            emit_func_body(&method->data->func_decl, ctx->source, ctx);
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
