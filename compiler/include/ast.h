#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "source_range.h"
#include "operator.h"

typedef struct Module Module;

typedef struct AST AST;

typedef struct IntLiteral
{
    u64 value;
} IntLiteral;

enum
{
    FRX_TYPE_KIND_ERROR,
    FRX_TYPE_KIND_UNRESOLVED,
    FRX_TYPE_KIND_PRIMITIVE,
    FRX_TYPE_KIND_ENUM,
    FRX_TYPE_KIND_STRUCT,
    FRX_TYPE_KIND_UNION,
    FRX_TYPE_KIND_POINTER,
    FRX_TYPE_KIND_ARRAY,

    FRX_TYPE_KIND_COUNT
};

typedef u8 TypeKind;

typedef struct TypeSpecifier TypeSpecifier;

typedef struct GenericArg
{
    AST* type;
} GenericArg;

typedef struct GenericArgs
{
    List args;
} GenericArgs;

typedef struct TypeSpecifier
{
    TypeKind kind;
    AST* generic_args;
    union
    {
        const char* name;
        TokenType primitive;
        usize size;
        struct
        {
            AST* base;
            b8 mutable;
        } ptr;
    };
} TypeSpecifier;

typedef struct UseStmt
{
    List path_segments;
    const char* symbol_name;
    Module* module;
} UseStmt;

typedef struct TraitBound
{
    AST* type;
} TraitBound;

typedef struct GenericParam
{
    const char* name;
    List trait_bounds;
} GenericParam;

typedef struct GenericParams
{
    List params;
} GenericParams;

typedef struct StructField
{
    const char* name;
    AST* type;
} StructField;

typedef struct StructDef
{
    const char* name;
    AST* generic_params;
    List fields;
} StructDef;

typedef struct EnumConstant
{
    const char* name;
} EnumConstant;

typedef struct EnumDef
{
    const char* name;
    AST* type;
    List constants;
} EnumDef;

typedef struct Trait
{
    const char* name;
    List methods;
} Trait;

typedef struct ImplBlock
{
    const char* type_name;
    List methods;
} ImplBlock;

typedef struct TranslationUnit
{
    List items;
} TranslationUnit;

typedef struct Scope
{
    List stmts;
} Scope;

typedef struct FuncParam
{
    const char* name;
    AST* type;
} FuncParam;

typedef struct FuncParams
{
    List params;
    b8 variadic;
} FuncParams;

typedef struct GenericInstantiation
{
    List concrete_types;
} GenericInstantiation;

typedef struct FuncDecl
{
    const char* name;
    AST* generic_params;
    AST* params;
    AST* return_type;
} FuncDecl;

typedef struct FuncDef
{
    const char* name;
    AST* generic_params;
    List generic_instantiations;
    AST* params;
    AST* return_type;
    AST* body;
} FuncDef;

typedef u8 ExprType;

typedef struct UnaryExpr
{
    TokenType type;
    Operator operator;
    AST* operand;
} UnaryExpr;

typedef struct BinaryExpr
{
    TokenType type;
    Operator operator;
    AST* left;
    AST* right;
} BinaryExpr;

typedef struct PathSegment
{
    const char* name;
    b8 external;
} PathSegment;

typedef struct PathExpr
{
    List path_segments;
} PathExpr;

typedef struct CallExpr
{
    List args;
} CallExpr;

typedef struct ExprStmt
{
    AST* expr;
} ExprStmt;

typedef struct BreakStmt
{
    const char* label;
} BreakStmt;

typedef struct ContinueStmt
{
    const char* label;
} ContinueStmt;

typedef struct ReturnStmt
{
    AST* value;
} ReturnStmt;

typedef struct IfStmt
{
    AST* condition;
    AST* if_block;
    AST* else_block;
} IfStmt;

typedef struct LetStmt
{
    b8 mutable;
    const char* name;
    AST* type;
    AST* value;
} LetStmt;

enum
{
    FRX_AST_TYPE_ERROR,
    FRX_AST_TYPE_TRANSLATION_UNIT,
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
    FRX_AST_TYPE_FUNC_PARAMS,
    FRX_AST_TYPE_GENERIC_PARAM,
    FRX_AST_TYPE_GENERIC_PARAMS,
    FRX_AST_TYPE_GENERIC_ARG,
    FRX_AST_TYPE_GENERIC_ARGS,
    FRX_AST_TYPE_FUNC_DECL,
    FRX_AST_TYPE_FUNC_DEF,
    FRX_AST_TYPE_SCOPE,
    FRX_AST_TYPE_EXPR_STMT,
    FRX_AST_TYPE_BREAK_STMT,
    FRX_AST_TYPE_CONTINUE_STMT,
    FRX_AST_TYPE_RETURN_STMT,
    FRX_AST_TYPE_LET_STMT,
    FRX_AST_TYPE_IF_STMT,
    FRX_AST_TYPE_UNARY_EXPR,
    FRX_AST_TYPE_BINARY_EXPR,
    FRX_AST_TYPE_PATH_SEGMENT,
    FRX_AST_TYPE_PATH_EXPR,
    FRX_AST_TYPE_CALL_EXPR,
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
        TranslationUnit translation_unit;
        UseStmt use_stmt;
        TypeSpecifier type_specifier;
        StructField struct_field;
        StructDef struct_def;
        EnumConstant enum_constant;
        EnumDef enum_def;
        Trait trait;
        TraitBound trait_bound;
        ImplBlock impl_block;
        FuncParam func_param;
        FuncParams func_params;
        GenericParam generic_param;
        GenericParams generic_params;
        GenericArg generic_arg;
        GenericArgs generic_args;
        FuncDecl func_decl;
        FuncDef func_def;
        Scope scope;
        ExprStmt expr_stmt;
        BreakStmt break_stmt;
        ContinueStmt continue_stmt;
        ReturnStmt return_stmt;
        LetStmt let_stmt;
        IfStmt if_stmt;
        UnaryExpr unary_expr;
        BinaryExpr binary_expr;
        PathSegment path_segment;
        PathExpr path_expr;
        CallExpr call_expr;
        IntLiteral int_literal;
    };
} AST;

AST* ast_create(ASTType type);

AST* scope_from_stmt(AST* stmt);

#endif
