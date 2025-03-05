#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "source_range.h"
#include "symbol_table.h"
#include "operator.h"

typedef struct IntLiteral
{
    u64 value;
    SourceRange range;
} IntLiteral;

enum
{
    FRX_ITEM_TYPE_FUNC_DEF,
    FRX_ITEM_TYPE_STRUCT_DEF,

    FRX_ITEM_TYPE_COUNT
};

typedef u8 ItemType;

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

typedef struct Item
{
    ItemType type;
    void* node;
    SourceRange range;
} Item;

typedef struct StructField
{
    const char* name;
    TypeSpecifier* type;
} StructField;

typedef struct StructDef
{
    SymbolID id;
    const char* name;
    List fields;
} StructDef;

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
    SymbolID id;
    const char* name;
    FuncParams* params;
    TypeSpecifier* return_type;
    Scope* body;
    SourceRange range;
} FuncDef;

enum
{
    FRX_EXPR_TYPE_INT_LIT,
    FRX_EXPR_TYPE_UNARY_EXPR,
    FRX_EXPR_TYPE_BINARY_EXPR,
    FRX_EXPR_TYPE_FUNC_CALL,
    FRX_EXPR_TYPE_VAR,

    FRX_EXPR_TYPE_COUNT
};

typedef u8 ExprType;

typedef struct Expr
{
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

typedef struct FuncCall
{
    const char* name;
    SymbolID id;
    List args;
} FuncCall;

typedef struct Var
{
    const char* name;
} Var;

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

typedef struct IfStmt
{
    Expr* condition;
    Scope* if_block;
    Scope* else_block;
} IfStmt;

enum
{
    FRX_STMT_TYPE_EXPR_STMT,
    FRX_STMT_TYPE_BREAK_STMT,
    FRX_STMT_TYPE_CONTINUE_STMT,
    FRX_STMT_TYPE_RETURN_STMT,
    FRX_STMT_TYPE_IF_STMT,

    FRX_STMT_TYPE_COUNT
};

typedef u8 StmtType;

typedef struct Stmt
{
    StmtType type;
    void* node;
    SourceRange range;
} Stmt;

Scope* scope_from_stmt(Stmt* stmt);

#endif
