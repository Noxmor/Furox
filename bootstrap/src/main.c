#include "lexer.h"

int main(void)
{
    Lexer lexer;
    lexer_init(&lexer, "input.txt"); 

    Token* token = NULL;
    while((token = lexer_peek(&lexer, 0))->type != FRX_TOKEN_TYPE_EOF)
    {
        if(token->type == FRX_TOKEN_TYPE_NUMBER)
            printf("Token: Type: %s (%zu)\n", token_type_to_str(token->type), token->number);
        else 
            printf("Token: Type: %s (%s)\n", token_type_to_str(token->type), token->identifier);

        lexer_next_token(&lexer);
    }

    lexer_destroy(&lexer);

    return 0;    
}
