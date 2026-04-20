#ifndef FRX_AST_H
#define FRX_AST_H

#include "types.h"
#include "token.h"
#include "list.h"
#include "operator.h"
#include "scope.h"
#include "module.h"
#include "source_file.h"

typedef struct AST AST;

typedef struct Type Type;

typedef u32 ASTLocalID;

typedef struct ASTNodeID
{
    SourceFileID owner;
    ASTLocalID local;
} ASTNodeID;

typedef struct ASTIntLiteral
{
    u64 value;
} ASTIntLiteral;

typedef struct ASTCharLiteral
{
    const char* value;
} ASTCharLiteral;

typedef struct ASTStringLiteral
{
    const char* value;
} ASTStringLiteral;

typedef struct ASTStructLiteralField
{
    const char* name;
    AST* value;
} ASTStructLiteralField;

typedef struct ASTStructLiteral
{
    AST* path;
    List fields;
} ASTStructLiteral;

typedef struct ASTTypeSpecifier ASTTypeSpecifier;

enum
{
    FRX_TYPE_SPECIFIER_KIND_PATH = 0,
    FRX_TYPE_SPECIFIER_KIND_FUNC,
    FRX_TYPE_SPECIFIER_KIND_PTR,
    FRX_TYPE_SPECIFIER_KIND_ARRAY,

    FRX_TYPE_SPECIFIER_KIND_COUNT
};

typedef u8 ASTTypeSpecifierKind;

typedef struct ASTTypeSpecifier
{
    ASTTypeSpecifierKind kind;
    const char* name;
    AST* path;
    List func_params;
    AST* func_return_type;
    b8 is_variadic;
    AST* base;
    AST* size;
    b8 mutable;
} ASTTypeSpecifier;

enum
{
    FRX_AST_USE_TREE_TYPE_SIMPLE = 0,
    FRX_AST_USE_TREE_TYPE_GLOB,
    FRX_AST_USE_TREE_TYPE_NESTED,

    FRX_AST_USE_TREE_TYPE_COUNT
};

typedef u8 ASTUseTreeType;

typedef struct ASTUseTree
{
    ASTUseTreeType type;
    const char* path_segment;
    List childs;
} ASTUseTree;

typedef struct ASTUseStmt
{
    AST* use_tree;
} ASTUseStmt;

typedef struct ASTTypeAlias
{
    const char* name;
    AST* type;
} ASTTypeAlias;

typedef struct ASTStatic
{
    b8 mutable;
    const char* name;
    AST* type;
    AST* value;
} ASTStatic;

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

enum
{
    FRX_STRUCT_KIND_NAMED,
    FRX_STRUCT_KIND_UNION,

    FRX_STRUCT_KIND_COUNT
};

typedef u8 StructKind;

typedef struct ASTStructField
{
    const char* name;
    SymbolVisibility visibility;
    AST* type;
} ASTStructField;

typedef struct ASTStructDef
{
    const char* name;
    StructKind kind;
    AST* generic_params;
    List fields;
    List instantiated_types;
} ASTStructDef;

typedef struct ASTEnumVariant
{
    const char* name;
    AST* value;
    Symbol* symbol;
} ASTEnumVariant;

typedef struct ASTEnumDef
{
    const char* name;
    AST* type;
    List variants;
} ASTEnumDef;

typedef struct ASTTrait
{
    const char* name;
    List methods;
} ASTTrait;

typedef struct ASTImplBlock
{
    AST* generic_params;
    AST* trait_path;
    AST* type_path;
    List methods;
} ASTImplBlock;

typedef struct ASTTranslationUnit
{
    AST* mod_decl;
    List items;
} ASTTranslationUnit;

typedef struct ASTBlock
{
    List stmts;
} ASTBlock;

enum
{
    FRX_FUNC_RECEIVER_NONE = 0,
    FRX_FUNC_RECEIVER_SELF_PTR,
    FRX_FUNC_RECEIVER_SELF_REF,

    FRX_FUNC_RECEIVER_COUNT
};

typedef u8 FuncReceiver;

typedef struct ASTFuncParam
{
    const char* name;
    AST* type;
} ASTFuncParam;

typedef struct ASTFuncDecl
{
    const char* name;
    b8 external;
    AST* generic_params;
    List params;
    FuncReceiver receiver;
    b8 is_variadic;
    AST* return_type;
    AST* body;
} ASTFuncDecl;

typedef struct ASTUnaryExpr
{
    TokenType type;
    Operator operator;
    AST* operand;
} ASTUnaryExpr;

typedef struct ASTBinaryExpr
{
    TokenType type;
    Operator operator;
    AST* left;
    AST* right;
} ASTBinaryExpr;

typedef struct ASTFieldExpr
{
    AST* base;
    const char* field_name;
    const Type* resolved_type; // TODO: Move this into an attributes table
} ASTFieldExpr;

typedef struct ASTSelfExpr
{
    b8 unused;
} ASTSelfExpr;

typedef struct ASTBoolExpr
{
    b8 value;
} ASTBoolExpr;

typedef struct ASTNullptrExpr
{
    b8 unused;
} ASTNullptrExpr;

typedef struct ASTCastExpr
{
    AST* expr;
    AST* type_specifier;
} ASTCastExpr;

typedef struct ASTSizeofExpr
{
    AST* expr;
} ASTSizeofExpr;

typedef struct ASTModDecl
{
    AST* path;
} ASTModDecl;

enum
{
    FRX_PATH_SEGMENT_TYPE_IDENT,
    FRX_PATH_SEGMENT_TYPE_PRIMITIVE,
    FRX_PATH_SEGMENT_TYPE_SELF_UPPER,

    FRX_PATH_SEGMENT_TYPE_COUNT
};

typedef u8 PathSegmentType;

typedef struct ASTPathSegment
{
    PathSegmentType type;
    const char* name;
    TokenType primitive;
    List generic_args;
} ASTPathSegment;

enum
{
    FRX_PATH_TYPE_ABSOLUTE,
    FRX_PATH_TYPE_EXTERN,
    FRX_PATH_TYPE_MOD,

    FRX_PATH_TYPE_COUNT
};

typedef u8 ASTPathType;

enum
{
    FRX_PATH_STYLE_EXPR,
    FRX_PATH_STYLE_TYPE,

    FRX_PATH_STYLE_COUNT
};

typedef u8 PathStyle;

typedef struct ASTPath
{
    ASTPathType type;
    List path_segments;
    Module* mod;
    const Symbol* symbol;
} ASTPath;

typedef struct ASTPathExpr
{
    AST* path;
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
    AST* then_block;
    AST* else_stmt;
} ASTIfStmt;

typedef struct ASTForLoop
{
    AST* init;
    AST* condition;
    AST* increment;
    AST* body;
} ASTForLoop;

typedef struct ASTWhileLoop
{
    AST* condition;
    AST* body;
} ASTWhileLoop;

typedef struct ASTLoop
{
    AST* body;
} ASTLoop;

typedef struct ASTLetStmt
{
    b8 mutable;
    const char* name;
    AST* type;
    AST* value;
} ASTLetStmt;

enum
{
    FRX_AST_TYPE_ERROR,
    FRX_AST_TYPE_TRANSLATION_UNIT,
    FRX_AST_TYPE_MOD_DECL,
    FRX_AST_TYPE_USE_TREE,
    FRX_AST_TYPE_USE_STMT,
    FRX_AST_TYPE_TYPE_SPECIFIER,
    FRX_AST_TYPE_TYPE_ALIAS,
    FRX_AST_TYPE_STATIC,
    FRX_AST_TYPE_STRUCT_FIELD,
    FRX_AST_TYPE_STRUCT_DEF,
    FRX_AST_TYPE_ENUM_VARIANT,
    FRX_AST_TYPE_ENUM_DEF,
    FRX_AST_TYPE_TRAIT,
    FRX_AST_TYPE_TRAIT_BOUND,
    FRX_AST_TYPE_IMPL_BLOCK,
    FRX_AST_TYPE_FUNC_PARAM,
    FRX_AST_TYPE_GENERIC_PARAM,
    FRX_AST_TYPE_GENERIC_PARAMS,
    FRX_AST_TYPE_FUNC_DECL,
    FRX_AST_TYPE_BLOCK,
    FRX_AST_TYPE_EXPR_STMT,
    FRX_AST_TYPE_BREAK_STMT,
    FRX_AST_TYPE_CONTINUE_STMT,
    FRX_AST_TYPE_RETURN_STMT,
    FRX_AST_TYPE_LET_STMT,
    FRX_AST_TYPE_IF_STMT,
    FRX_AST_TYPE_FOR_LOOP,
    FRX_AST_TYPE_WHILE_LOOP,
    FRX_AST_TYPE_LOOP,
    FRX_AST_TYPE_UNARY_EXPR,
    FRX_AST_TYPE_BINARY_EXPR,
    FRX_AST_TYPE_FIELD_EXPR,
    FRX_AST_TYPE_SELF_EXPR,
    FRX_AST_TYPE_BOOL_EXPR,
    FRX_AST_TYPE_NULLPTR_EXPR,
    FRX_AST_TYPE_PATH_SEGMENT,
    FRX_AST_TYPE_PATH,
    FRX_AST_TYPE_PATH_EXPR,
    FRX_AST_TYPE_CALL_EXPR,
    FRX_AST_TYPE_METHOD_CALL_EXPR,
    FRX_AST_TYPE_CAST_EXPR,
    FRX_AST_TYPE_SIZEOF_EXPR,
    FRX_AST_TYPE_INT_LIT,
    FRX_AST_TYPE_CHAR_LIT,
    FRX_AST_TYPE_STRING_LIT,
    FRX_AST_TYPE_STRUCT_LIT,
    FRX_AST_TYPE_STRUCT_LIT_FIELD,

    FRX_AST_TYPE_COUNT
};

typedef u8 ASTType;

typedef struct AST
{
    ASTNodeID id;
    SourceSpan span;
    ASTType type;
    union
    {
        ASTTranslationUnit translation_unit;
        ASTModDecl mod_decl;
        ASTUseTree use_tree;
        ASTUseStmt use_stmt;
        ASTTypeSpecifier type_specifier;
        ASTTypeAlias type_alias;
        ASTStatic static_node;
        ASTStructField struct_field;
        ASTStructDef struct_def;
        ASTEnumVariant enum_variant;
        ASTEnumDef enum_def;
        ASTTrait trait;
        ASTTraitBound trait_bound;
        ASTImplBlock impl_block;
        ASTFuncParam func_param;
        ASTGenericParam generic_param;
        ASTGenericParams generic_params;
        ASTFuncDecl func_decl;
        ASTBlock block;
        ASTExprStmt expr_stmt;
        ASTBreakStmt break_stmt;
        ASTContinueStmt continue_stmt;
        ASTReturnStmt return_stmt;
        ASTLetStmt let_stmt;
        ASTIfStmt if_stmt;
        ASTForLoop for_loop;
        ASTWhileLoop while_loop;
        ASTLoop loop;
        ASTUnaryExpr unary_expr;
        ASTBinaryExpr binary_expr;
        ASTFieldExpr field_expr;
        ASTSelfExpr self_expr;
        ASTBoolExpr bool_expr;
        ASTNullptrExpr nullptr_expr;
        ASTPathSegment path_segment;
        ASTPath path;
        ASTPathExpr path_expr;
        ASTCallExpr call_expr;
        ASTMethodCallExpr method_call_expr;
        ASTCastExpr cast_expr;
        ASTSizeofExpr sizeof_expr;
        ASTIntLiteral int_literal;
        ASTCharLiteral char_literal;
        ASTStringLiteral string_literal;
        ASTStructLiteral struct_literal;
        ASTStructLiteralField struct_literal_field;
    };
} AST;

AST* ast_create(ASTType type, ASTNodeID id);

const Type* expr_infer_type(AST* expr);

AST* enum_def_lookup_variant(ASTEnumDef* enum_def, const char* name);

#endif
