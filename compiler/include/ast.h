#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "source_range.h"
#include "operator.h"

typedef struct Module Module;

typedef struct IntLiteral
{
    b8 error;
    u64 value;
    SourceRange range;
} IntLiteral;

enum
{
    FRX_ITEM_TYPE_ERROR,
    FRX_ITEM_TYPE_USE_STMT,
    FRX_ITEM_TYPE_FUNC_DECL,
    FRX_ITEM_TYPE_FUNC_DEF,
    FRX_ITEM_TYPE_STRUCT_DEF,
    FRX_ITEM_TYPE_ENUM_DEF,
    FRX_ITEM_TYPE_TRAIT,
    FRX_ITEM_TYPE_IMPL_BLOCK,

    FRX_ITEM_TYPE_COUNT
};

typedef u8 ItemType;

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
    TypeSpecifier* type;
} GenericArg;

typedef struct GenericArgs
{
    b8 error;
    List args;
} GenericArgs;

typedef struct TypeSpecifier
{
    b8 error;
    TypeKind kind;
    GenericArgs* generic_args;
    union
    {
        const char* name;
        TokenType primitive;
        usize size;
        struct
        {
            struct TypeSpecifier* base;
            b8 mutable;
        } ptr;
    };
} TypeSpecifier;

typedef struct Item
{
    b8 error;
    ItemType type;
    void* node;
    SourceRange range;
} Item;

typedef struct UseStmt
{
    b8 error;
    List path_segments;
    const char* symbol_name;
    Module* module;
} UseStmt;

typedef struct TraitBound
{
    TypeSpecifier* type;
} TraitBound;

typedef struct GenericParam
{
    const char* name;
    List trait_bounds;
} GenericParam;

typedef struct GenericParams
{
    b8 error;
    List params;
} GenericParams;

typedef struct StructField
{
    b8 error;
    const char* name;
    TypeSpecifier* type;
} StructField;

typedef struct StructDef
{
    b8 error;
    const char* name;
    GenericParams* generic_params;
    List fields;
} StructDef;

typedef struct EnumConstant
{
    const char* name;
} EnumConstant;

typedef struct EnumDef
{
    const char* name;
    TypeSpecifier* type;
    List constants;
} EnumDef;

typedef struct Trait
{
    b8 error;
    const char* name;
    List methods;
} Trait;

typedef struct ImplBlock
{
    b8 error;
    const char* type_name;
    List methods;
} ImplBlock;

typedef struct TranslationUnit
{
    b8 error;
    List items;
    SourceRange range;
} TranslationUnit;

typedef struct Scope
{
    b8 error;
    List stmts;
    SourceRange range;
} Scope;

typedef struct FuncParam
{
    b8 error;
    const char* name;
    TypeSpecifier* type;
} FuncParam;

typedef struct FuncParams
{
    b8 error;
    List params;
    b8 variadic;
} FuncParams;

typedef struct GenericInstantiation
{
    List concrete_types;
} GenericInstantiation;

typedef struct FuncDecl
{
    b8 error;
    const char* name;
    GenericParams* generic_params;
    FuncParams* params;
    TypeSpecifier* return_type;
    SourceRange range;
} FuncDecl;

typedef struct FuncDef
{
    b8 error;
    const char* name;
    GenericParams* generic_params;
    List generic_instantiations;
    FuncParams* params;
    TypeSpecifier* return_type;
    Scope* body;
    SourceRange range;
} FuncDef;

enum
{
    FRX_EXPR_TYPE_ERROR,
    FRX_EXPR_TYPE_INT_LIT,
    FRX_EXPR_TYPE_UNARY_EXPR,
    FRX_EXPR_TYPE_BINARY_EXPR,
    FRX_EXPR_TYPE_PATH_EXPR,
    FRX_EXPR_TYPE_CALL_EXPR,

    FRX_EXPR_TYPE_COUNT
};

typedef u8 ExprType;

typedef struct Expr
{
    b8 error;
    ExprType type;
    void* node;
    SourceRange range;
} Expr;

typedef struct UnaryExpr
{
    TokenType type;
    Operator operator;
    Expr* operand;
} UnaryExpr;

typedef struct BinaryExpr
{
    TokenType type;
    Operator operator;
    Expr* left;
    Expr* right;
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
    b8 error;
    Expr* expr;
    SourceRange range;
} ExprStmt;

typedef struct BreakStmt
{
    b8 error;
    SourceRange range;
} BreakStmt;

typedef struct ContinueStmt
{
    b8 error;
    SourceRange range;
} ContinueStmt;

typedef struct ReturnStmt
{
    b8 error;
    Expr* value;
    SourceRange range;
} ReturnStmt;

typedef struct IfStmt
{
    b8 error;
    Expr* condition;
    Scope* if_block;
    Scope* else_block;
} IfStmt;

enum
{
    FRX_STMT_TYPE_ERROR,
    FRX_STMT_TYPE_EXPR_STMT,
    FRX_STMT_TYPE_BREAK_STMT,
    FRX_STMT_TYPE_CONTINUE_STMT,
    FRX_STMT_TYPE_RETURN_STMT,
    FRX_STMT_TYPE_LET_STMT,
    FRX_STMT_TYPE_IF_STMT,

    FRX_STMT_TYPE_COUNT
};

typedef u8 StmtType;

typedef struct Stmt
{
    b8 error;
    StmtType type;
    void* node;
    SourceRange range;
} Stmt;

typedef struct LetStmt
{
    b8 error;
    b8 mutable;
    const char* name;
    TypeSpecifier* type;
    Expr* value;
} LetStmt;

Scope* scope_from_stmt(Stmt* stmt);

#endif
