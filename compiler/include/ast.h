#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "source_range.h"

typedef struct IntLiteral
{
    u64 value;
    SourceRange range;
} IntLiteral;

enum
{
    FRX_ITEM_TYPE_FUNC_DEF,

    FRX_ITEM_TYPE_COUNT
};

enum
{
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

typedef struct TypeSpecifier
{
    TypeKind kind;
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

typedef u8 ItemType;

typedef struct Item
{
    ItemType type;
    void* node;
    SourceRange range;
} Item;

typedef struct TranslationUnit
{
    List items;
    SourceRange range;
} TranslationUnit;

typedef struct Scope
{
    List stmts;
    SourceRange range;
} Scope;

typedef struct FuncParam
{
    const char* name;
    TypeSpecifier* type;
} FuncParam;

typedef struct FuncParams
{
    List params;
    b8 variadic;
} FuncParams;

typedef struct FuncDef
{
    const char* name;
    FuncParams* params;
    TypeSpecifier* return_type;
    Scope* body;
    SourceRange range;
} FuncDef;

enum
{
    FRX_EXPR_TYPE_INT_LIT,

    FRX_EXPR_TYPE_COUNT
};

typedef u8 ExprType;

typedef struct Expr
{
    ExprType type;
    void* node;
    SourceRange range;
} Expr;

typedef struct ExprStmt
{
    Expr* expr;
    SourceRange range;
} ExprStmt;

typedef struct BreakStmt
{
    SourceRange range;
} BreakStmt;

typedef struct ContinueStmt
{
    SourceRange range;
} ContinueStmt;

typedef struct ReturnStmt
{
    Expr* value;
    SourceRange range;
} ReturnStmt;

enum
{
    FRX_STMT_TYPE_EXPR_STMT,
    FRX_STMT_TYPE_BREAK_STMT,
    FRX_STMT_TYPE_CONTINUE_STMT,
    FRX_STMT_TYPE_RETURN_STMT,

    FRX_STMT_TYPE_COUNT
};

typedef u8 StmtType;

typedef struct Stmt
{
    StmtType type;
    void* node;
    SourceRange range;
} Stmt;

#endif
