#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "source_range.h"
#include "operator.h"
#include "type_system.h"
#include "scope.h"
#include "module.h"

typedef struct AST AST;

typedef struct ASTIntLiteral
{
    u64 value;
    Type* resolved_type;
} ASTIntLiteral;

typedef struct ASTTypeSpecifier ASTTypeSpecifier;

typedef struct ASTGenericArg
{
    AST* type;
} ASTGenericArg;

typedef struct ASTGenericArgs
{
    List args;
} ASTGenericArgs;

enum
{
    FRX_TYPE_SPECIFIER_KIND_PRIMITIVE = 0,
    FRX_TYPE_SPECIFIER_KIND_PATH_EXPR,
    FRX_TYPE_SPECIFIER_KIND_PTR,
    FRX_TYPE_SPECIFIER_KIND_ARRAY,

    FRX_TYPE_SPECIFIER_KIND_COUNT
};

typedef u8 ASTTypeSpecifierKind;

typedef struct ASTTypeSpecifier
{
    AST* generic_args;
    ASTTypeSpecifierKind kind;
    const char* name;
    TokenType primitive;
    AST* path_expr;
    usize size;
    AST* base;
    b8 mutable;
    Type* resolved_type;
} ASTTypeSpecifier;

typedef struct ASTUseStmt
{
    List path_segments;
    const char* symbol_name;
} ASTUseStmt;

typedef struct ASTTraitBound
{
    AST* type;
} ASTTraitBound;

typedef struct ASTGenericParam
{
    const char* name;
    List trait_bounds;
} ASTGenericParam;

typedef struct ASTGenericParams
{
    List params;
} ASTGenericParams;

typedef struct ASTStructField
{
    const char* name;
    AST* type;
} ASTStructField;

typedef struct ASTStructDef
{
    const char* name;
    AST* generic_params;
    List fields;
    Type* resolved_type;
} ASTStructDef;

typedef struct ASTEnumConstant
{
    const char* name;
} ASTEnumConstant;

typedef struct ASTEnumDef
{
    const char* name;
    AST* type;
    List constants;
    Type* resolved_type;
} ASTEnumDef;

typedef struct ASTTrait
{
    const char* name;
    List methods;
} ASTTrait;

typedef struct ASTImplBlock
{
    Scope* scope;
    TokenType primitive;
    AST* path_expr;
    List methods;
} ASTImplBlock;

typedef struct ASTTranslationUnit
{
    AST* mod_decl;
    List items;
} ASTTranslationUnit;

typedef struct ASTScope
{
    Scope* scope;
    List stmts;
} ASTScope;

typedef struct ASTFuncParam
{
    const char* name;
    AST* type;
} ASTFuncParam;

typedef struct ASTFuncDecl
{
    Scope* scope;
    const char* name;
    AST* generic_params;
    List params;
    b8 is_variadic;
    AST* return_type;
    AST* body;
} ASTFuncDecl;

typedef u8 ASTExprType;

typedef struct ASTUnaryExpr
{
    TokenType type;
    Operator operator;
    AST* operand;
    Type* resolved_type;
} ASTUnaryExpr;

typedef struct ASTBinaryExpr
{
    TokenType type;
    Operator operator;
    AST* left;
    AST* right;
    Type* resolved_type;
} ASTBinaryExpr;

typedef struct ASTFieldExpr
{
    AST* base;
    const char* field_name;
    Type* resolved_type;
} ASTFieldExpr;

typedef struct ASTModDecl
{
    AST* path_expr;
} ASTModDecl;

enum
{
    FRX_PATH_TYPE_ABSOLUTE,
    FRX_PATH_TYPE_EXTERN,
    FRX_PATH_TYPE_MOD,

    FRX_PATH_TYPE_COUNT
};

typedef u8 ASTPathType;

typedef struct ASTPathExpr
{
    ASTPathType type;
    List path_segments;
    Scope* scope;
    Module* mod;
    const Symbol* symbol;
} ASTPathExpr;

typedef struct ASTCallExpr
{
    AST* callee;
    List args;
} ASTCallExpr;

typedef struct ASTMethodCallExpr
{
    AST* callee;
    const char* name;
    List args;
    Type* resolved_type;
    const Symbol* symbol;
} ASTMethodCallExpr;

typedef struct ASTExprStmt
{
    AST* expr;
} ASTExprStmt;

typedef struct ASTBreakStmt
{
    const char* label;
} ASTBreakStmt;

typedef struct ASTContinueStmt
{
    const char* label;
} ASTContinueStmt;

typedef struct ASTReturnStmt
{
    AST* value;
} ASTReturnStmt;

typedef struct ASTIfStmt
{
    AST* condition;
    AST* if_block;
    AST* else_block;
} ASTIfStmt;

typedef struct ASTLetStmt
{
    b8 mutable;
    const char* name;
    AST* type;
    AST* value;
    Type* resolved_type;
} ASTLetStmt;

enum
{
    FRX_AST_TYPE_ERROR,
    FRX_AST_TYPE_TRANSLATION_UNIT,
    FRX_AST_TYPE_MOD_DECL,
    FRX_AST_TYPE_USE_STMT,
    FRX_AST_TYPE_TYPE_SPECIFIER,
    FRX_AST_TYPE_STRUCT_FIELD,
    FRX_AST_TYPE_STRUCT_DEF,
    FRX_AST_TYPE_ENUM_CONSTANT,
    FRX_AST_TYPE_ENUM_DEF,
    FRX_AST_TYPE_TRAIT,
    FRX_AST_TYPE_TRAIT_BOUND,
    FRX_AST_TYPE_IMPL_BLOCK,
    FRX_AST_TYPE_FUNC_PARAM,
    FRX_AST_TYPE_GENERIC_PARAM,
    FRX_AST_TYPE_GENERIC_PARAMS,
    FRX_AST_TYPE_GENERIC_ARG,
    FRX_AST_TYPE_GENERIC_ARGS,
    FRX_AST_TYPE_FUNC_DECL,
    FRX_AST_TYPE_SCOPE,
    FRX_AST_TYPE_EXPR_STMT,
    FRX_AST_TYPE_BREAK_STMT,
    FRX_AST_TYPE_CONTINUE_STMT,
    FRX_AST_TYPE_RETURN_STMT,
    FRX_AST_TYPE_LET_STMT,
    FRX_AST_TYPE_IF_STMT,
    FRX_AST_TYPE_UNARY_EXPR,
    FRX_AST_TYPE_BINARY_EXPR,
    FRX_AST_TYPE_FIELD_EXPR,
    FRX_AST_TYPE_PATH_EXPR,
    FRX_AST_TYPE_CALL_EXPR,
    FRX_AST_TYPE_METHOD_CALL_EXPR,
    FRX_AST_TYPE_INT_LIT,

    FRX_AST_TYPE_COUNT
};

typedef u8 ASTType;

typedef struct AST
{
    SourceRange range;
    ASTType type;
    union
    {
        ASTTranslationUnit translation_unit;
        ASTModDecl mod_decl;
        ASTUseStmt use_stmt;
        ASTTypeSpecifier type_specifier;
        ASTStructField struct_field;
        ASTStructDef struct_def;
        ASTEnumConstant enum_constant;
        ASTEnumDef enum_def;
        ASTTrait trait;
        ASTTraitBound trait_bound;
        ASTImplBlock impl_block;
        ASTFuncParam func_param;
        ASTGenericParam generic_param;
        ASTGenericParams generic_params;
        ASTGenericArg generic_arg;
        ASTGenericArgs generic_args;
        ASTFuncDecl func_decl;
        ASTScope scope;
        ASTExprStmt expr_stmt;
        ASTBreakStmt break_stmt;
        ASTContinueStmt continue_stmt;
        ASTReturnStmt return_stmt;
        ASTLetStmt let_stmt;
        ASTIfStmt if_stmt;
        ASTUnaryExpr unary_expr;
        ASTBinaryExpr binary_expr;
        ASTFieldExpr field_expr;
        ASTPathExpr path_expr;
        ASTCallExpr call_expr;
        ASTMethodCallExpr method_call_expr;
        ASTIntLiteral int_literal;
    };
} AST;

AST* ast_create(ASTType type);

AST* scope_from_stmt(AST* stmt);

Type* expr_infer_type(AST* expr);

#endif
