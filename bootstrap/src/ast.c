#include "ast.h"

#include "core/assert.h"
#include "core/memory.h"

const char* ast_type_to_str(ASTType type)
{
    FRX_ASSERT(type < FRX_AST_TYPE_COUNT);

    switch(type)
    {
        case FRX_AST_TYPE_NOOP: return "Noop";

        case FRX_AST_TYPE_COMPOUND: return "Compound";

        case FRX_AST_TYPE_NUMBER: return "Number";

        case FRX_AST_TYPE_CHAR_LITERAL: return "Char Literal";
        case FRX_AST_TYPE_STRING_LITERAL: return "String Literal";

        case FRX_AST_TYPE_VARIABLE_DECLARATION: return "Variable Declaration";
        case FRX_AST_TYPE_VARIABLE_DEFINITION: return "Variable Definition";
        case FRX_AST_TYPE_VARIABLE: return "Variable";

        case FRX_AST_TYPE_ADDITION: return "Addition";
        case FRX_AST_TYPE_SUBTRACTION: return "Subtraction";
        case FRX_AST_TYPE_MULTIPLICATION: return "Multiplication";
        case FRX_AST_TYPE_DIVISION: return "Division";

        case FRX_AST_TYPE_RETURN_STATEMENT: return "Return Statement";

        case FRX_AST_TYPE_PARAMETER_LIST: return "Parameter List";
        case FRX_AST_TYPE_FUNCTION_DEFINITION: return "Function Definition";
        case FRX_AST_TYPE_FUNCTION_CALL: return "Function Call";

        case FRX_AST_TYPE_SCOPE: return "Scope";

        default: FRX_ASSERT(FRX_FALSE);
    }

    return "Unknown";
}

void ast_init(AST* ast, ASTType type)
{
    FRX_ASSERT(type < FRX_AST_TYPE_COUNT);

    ast->type = type;

    ast->data = NULL;

    ast->children_size = 0;
    ast->children_capacity = 0;
    ast->children = NULL;
}

AST* ast_new_child(AST* parent, ASTType child_type)
{
    FRX_ASSERT(parent != NULL);

    if(parent->children_size >= parent->children_capacity)
    {
        parent->children_capacity = parent->children_capacity == 0 ? 1: parent->children_capacity * 2;
        parent->children = memory_realloc(parent->children, parent->children_capacity * sizeof(AST), FRX_MEMORY_CATEGORY_AST);
    }

    AST* child = &parent->children[parent->children_size++];

    ast_init(child, child_type);

    return child;
}

#define FRX_AST_PRINT_RECURSION_LIMIT 128
static u8 recursion_buffer[FRX_AST_PRINT_RECURSION_LIMIT];

static void ast_print_recursive(const AST* root, usize depth)
{
    FRX_ASSERT(root != NULL);

    FRX_ASSERT(depth < FRX_AST_PRINT_RECURSION_LIMIT);

    for(usize i = 0; i < depth; ++i)
    {
        if(i < depth - 1)
            printf("%c   ", recursion_buffer[i] ? '|' : ' ');
        else
            printf("+---");
    }

    printf("%s\n", ast_type_to_str(root->type));

    recursion_buffer[depth] = 1;

    for(usize i = 0; i < root->children_size; ++i)
    {
        if(i == root->children_size - 1)
            recursion_buffer[depth] = 0;
        
        for(usize j = 0; j < depth; ++j)
        {
            printf("%c   ", recursion_buffer[j] ? '|' : ' ');
        }

        printf("|\n");

        ast_print_recursive(&root->children[i], depth + 1);
    }
}

void ast_print(const AST* root)
{
    FRX_ASSERT(root != NULL);

    ast_print_recursive(root, 0);
}
