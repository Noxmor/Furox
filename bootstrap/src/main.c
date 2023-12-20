#include "ast.h"

int main(void)
{
    AST root;
    
    ast_init(&root, FRX_AST_TYPE_COMPOUND);
    
    AST* func_def = ast_new_child(&root, FRX_AST_TYPE_FUNCTION_DEFINITION);
    AST* param_list = ast_new_child(func_def, FRX_AST_TYPE_PARAMETER_LIST);
    AST* foo = ast_new_child(param_list, FRX_AST_TYPE_NOOP);
    AST* bar = ast_new_child(param_list, FRX_AST_TYPE_NOOP);
    AST* number = ast_new_child(func_def, FRX_AST_TYPE_NUMBER);

    ast_print(&root);

    return 0;    
}
