#include "transpiler.h"

#include <ctype.h>
#include <string.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "core/assert.h"
#include "core/memory.h"

#define FRX_TRANSPILER_ABORT_ON_ERROR(expr) if(expr) return FRX_TRUE
#define FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(file, format, ...) if(fprintf(file, format, ##__VA_ARGS__) < 0) return FRX_TRUE

typedef struct TranspilerInfo
{
    char** c_filepaths;
    usize c_filepaths_size;
    usize c_filepaths_capacity;
} TranspilerInfo;

static TranspilerInfo transpiler_info;

typedef struct Namespace
{
    const char* namespace;
    struct Namespace* next;
} Namespace;

static void store_c_filepath(const char* c_filepath)
{
    if(transpiler_info.c_filepaths_size >= transpiler_info.c_filepaths_capacity)
    {
        transpiler_info.c_filepaths_capacity = transpiler_info.c_filepaths_capacity == 0 ? 1 : transpiler_info.c_filepaths_capacity * 2;
        transpiler_info.c_filepaths = memory_realloc(transpiler_info.c_filepaths, transpiler_info.c_filepaths_capacity * sizeof(char*), FRX_MEMORY_CATEGORY_STRING);
    }

    transpiler_info.c_filepaths[transpiler_info.c_filepaths_size++] = strdup(c_filepath);
}

static Namespace* current_namespace = NULL;

static FRX_NO_DISCARD b8 write_current_namespace(FILE* f)
{
    Namespace* namespace = current_namespace;

    while(namespace != NULL)
    {
        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s_", namespace->namespace);
        namespace = namespace->next;
    }

    return FRX_FALSE;
}

static Namespace* get_last_namespace(void)
{
    Namespace* namespace = current_namespace;

    if(namespace == NULL)
        return NULL;

    while(namespace->next != NULL)
        namespace = namespace->next;

    return namespace;
}

static void append_namespace(const char* namespace)
{
    Namespace* last = get_last_namespace();

    Namespace* new = memory_alloc(sizeof(Namespace), FRX_MEMORY_CATEGORY_UNKNOWN);
    
    if(last == NULL)
        current_namespace = new;
    else
        last->next = new;

    new->namespace = namespace;
    new->next = NULL;
}

static void drop_namespace(void)
{
    FRX_ASSERT(current_namespace != NULL);

    if(current_namespace->next == NULL)
    {
        memory_free(current_namespace);

        current_namespace = NULL;

        return;
    }

    Namespace* namespace = current_namespace;

    while(namespace->next != NULL && namespace->next->next != NULL)
    {
        namespace = namespace->next;
    }

    memory_free(namespace->next);

    namespace->next = NULL;
}

static usize indentation_level = 0;

static void push_indentation_level(void)
{
    ++indentation_level;
}

static void drop_indentation_level(void)
{
    FRX_ASSERT(indentation_level > 0);

    --indentation_level;
}

static FRX_NO_DISCARD b8 print_indentation_level(FILE* f)
{
    for(usize i = 0; i < indentation_level; ++i)
        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "    ");

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 transpile_c(const AST* root, FILE* f)
{
    FRX_ASSERT(root != NULL);

    FRX_ASSERT(f != NULL);

    switch(root->type)
    {
        case FRX_AST_TYPE_NOOP: break;
        
        case FRX_AST_TYPE_COMPOUND:
        {
            for(usize i = 0; i < root->children_size; ++i)
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[i], f));

            break;
        }

        case FRX_AST_TYPE_NUMBER:
        {
            NumberData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%zu", data->number);

            break;
        }

        case FRX_AST_TYPE_CHAR_LITERAL:
        {
            CharLiteralData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "'%s'", data->literal);

            break;
        }

        case FRX_AST_TYPE_STRING_LITERAL:
        {
            StringLiteralData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "\"%s\"", data->literal);

            break;
        }

        case FRX_AST_TYPE_TYPE:
        {
            TypeData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s", data->name);

            for(usize i = 0; i < data->pointer_level; ++i)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "*");
            }

            break;
        }

        case FRX_AST_TYPE_VARIABLE_DECLARATION:
        {
            VariableData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %s", data->name);

            AST* type = &root->children[0];
            while(type->type != FRX_AST_TYPE_TYPE)
                type = &type->children[0];

            if(type->children_size > 0)
            {
                AST* array_size_expr = &type->children[0];

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "[");
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(array_size_expr, f));
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "];\n");
            }
            else
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ";\n");
            }

            break;
        }

        case FRX_AST_TYPE_VARIABLE_DEFINITION:
        {
            VariableData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %s", data->name);

            AST* type = &root->children[0];
            while(type->type != FRX_AST_TYPE_TYPE)
                type = &type->children[0];

            if(type->children_size > 0)
            {
                AST* array_size_expr = &type->children[0];

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "[");
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(array_size_expr, f));
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "] = ");
            }
            else
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " = ");
            }

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ";\n");

            break;
        }

        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT:
        {
            AST* variable = &root->children[0];
            AST* expr = &root->children[1];

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(variable, f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " = ");

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(expr, f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ";\n");

            break;
        }

        case FRX_AST_TYPE_VARIABLE:
        {
            VariableData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s", data->name);

            if(root->children[0].type != FRX_AST_TYPE_NOOP)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "[");
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0].children[0], f));
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "]");
            }

            if(root->children_size > 1)
            {
                if(data->is_pointer)
                {
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "->");
                }
                else
                {
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ".");
                }

                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            }

            break;
        }

        case FRX_AST_TYPE_ADDITION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " + ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_SUBTRACTION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " - ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        
        case FRX_AST_TYPE_MULTIPLICATION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " * ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_DIVISION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " / ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_MODULO:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %% ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_ARITHMETIC_NEGATION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "-");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_LOGICAL_AND:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " && ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_LOGICAL_OR:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " || ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_LOGICAL_NEGATION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "!");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_AND:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " & ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_OR:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " | ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_XOR:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " ^ ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_NEGATION:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "~");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_LEFT_SHIFT:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " << ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_BINARY_RIGHT_SHIFT:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " >> ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }
        case FRX_AST_TYPE_COMPARISON:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " == ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        
        }
        case FRX_AST_TYPE_GREATER_THAN:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " > ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_GREATER_THAN_EQUALS:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " >= ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_LESS_THAN:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " < ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_LESS_THAN_EQUALS:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " <= ");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_DEREFERENCE:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "*(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_ADDRESS_OF:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "&(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            break;
        }

        case FRX_AST_TYPE_IF_STATEMENT:
        {
            break;
        }

        case FRX_AST_TYPE_FOR_LOOP:
        {
            break;
        }

        case FRX_AST_TYPE_WHILE_LOOP:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "while(");
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")\n");

            FRX_TRANSPILER_ABORT_ON_ERROR(print_indentation_level(f));

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[1], f));

            break;
        }

        case FRX_AST_TYPE_DO_WHILE_LOOP:
        {
            break;
        }

        case FRX_AST_TYPE_RETURN_STATEMENT:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "return");

            if(root->children_size > 0)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " ");
                
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            }

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ";\n");

            break;
        }

        case FRX_AST_TYPE_FUNCTION_DEFINITION:
        {
            FunctionDefinitionData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " ");

            FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);
            
            AST* parameter_list = &root->children[1];
            
            for(usize i = 0; i < parameter_list->children_size; ++i)
            {
                AST* parameter = &parameter_list->children[i];

                VariableData* parameter_data = parameter->data;

                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&parameter->children[0], f));

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %s", parameter_data->name);

                if(i != parameter_list->children_size - 1)
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
            }

            if(parameter_list->children_size == 0 && !data->is_variadic)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "void");
            }

            if(data->is_variadic)
            {
                if(parameter_list->children_size > 0)
                {
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
                }

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "...");
            }

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")\n");

            return transpile_c(&root->children[2], f);
        }

        case FRX_AST_TYPE_FUNCTION_DECLARATION:
        {
            FunctionDeclarationData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " ");

            FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);

            AST* parameter_list = &root->children[1];

            for(usize i = 0; i < parameter_list->children_size; ++i)
            {
                AST* parameter = &parameter_list->children[i];

                VariableData* parameter_data = parameter->data;

                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&parameter->children[0], f));

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %s", parameter_data->name);

                if(i != parameter_list->children_size - 1)
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
            }

            if(parameter_list->children_size == 0 && !data->is_variadic)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "void");
            }

            if(data->is_variadic)
            {
                if(parameter_list->children_size > 0)
                {
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
                }

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "...");
            }

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ");\n");

            break;
        }

        case FRX_AST_TYPE_FUNCTION_CALL:
        {
            FunctionCallData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);

            for(usize i = 0; i < root->children_size; ++i)
            {
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[i], f));

                if(i != root->children_size - 1)
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
            }

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")");

            if(data->is_statement)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ";\n");
            }

            break;
        }

        case FRX_AST_TYPE_SCOPE:
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "{\n");
           
            push_indentation_level();

            for(usize i = 0; i < root->children_size; ++i)
            {
                FRX_TRANSPILER_ABORT_ON_ERROR(print_indentation_level(f));
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[i], f));
            }

            drop_indentation_level();
            FRX_TRANSPILER_ABORT_ON_ERROR(print_indentation_level(f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "}\n\n");

            break;
        }

        case FRX_AST_TYPE_STRUCT_DEFINITION: break;

        case FRX_AST_TYPE_NAMESPACE:
        {
            NamespaceData* data = root->data;

            append_namespace(data->namespace);

            for(usize i = 0; i < root->children_size; ++i)
                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[i], f));

            drop_namespace();

            break;
        }

        case FRX_AST_TYPE_NAMESPACE_REF:
        {
            NamespaceData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s_", data->namespace);

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

            break;
        }

        case FRX_AST_TYPE_EXTERN_BLOCK:
        {
            for(usize i = 0; i < root->children_size; ++i)
            {
                AST* child = &root->children[i];

                if(child->type == FRX_AST_TYPE_FUNCTION_DECLARATION)
                {
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "extern ");

                    FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(child, f));
                }
            }

            break;
        }

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 transpile_header(const AST* root, FILE* f)
{
    FRX_ASSERT(root != NULL);

    FRX_ASSERT(f != NULL);

    if(root->type == FRX_AST_TYPE_NAMESPACE)
    {
        NamespaceData* data = root->data;

        append_namespace(data->namespace);

        for(usize i = 0; i < root->children_size; ++i)
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_header(&root->children[i], f));

        drop_namespace();

        return FRX_FALSE;
    }

    if(root->type == FRX_AST_TYPE_FUNCTION_DEFINITION)
    {
        FunctionDefinitionData* data = root->data;

        FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " ");

        FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));
        
        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);
        
        AST* parameter_list = &root->children[1];

        for(usize i = 0; i < parameter_list->children_size; ++i)
        {
            AST* parameter = &parameter_list->children[i];

            VariableData* parameter_data = parameter->data;

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&parameter->children[0], f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, " %s", parameter_data->name);

            if(i != parameter_list->children_size - 1)
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
        }

        if(parameter_list->children_size == 0 && !data->is_variadic)
        {
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "void");
        }

        if(data->is_variadic)
        {
            if(parameter_list->children_size > 0)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
            }

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "...");
        }

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ");\n\n");
    }
    else if(root->type == FRX_AST_TYPE_STRUCT_DEFINITION)
    {
        StructDefinitionData* data = root->data;

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "typedef struct ");

        FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s\n{\n", data->name);

        for(usize i = 0; i < root->children_size; ++i)
        {
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[i], f));
        }

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "} ");

        FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s;\n\n", data->name);
    }
    else
    {
        for(usize i = 0; i < root->children_size; ++i)
            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_header(&root->children[i], f));
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 create_furox_directory_from_source(const char* filepath, char* furox_filepath)
{
    FRX_ASSERT(filepath != NULL);

    FRX_ASSERT(furox_filepath != NULL);

    const char* last_slash = strrchr(filepath, '/');

    if(last_slash == NULL)
        strcpy(furox_filepath, "furox-c");
    else
    {
        usize path_len = last_slash - filepath;

        FRX_ASSERT(path_len < 1000);

        sprintf(furox_filepath, "furox-c/%.*s", (int)path_len, filepath);
    }

    return mkdir(furox_filepath, 0777) == -1 && errno != EEXIST;
}

static FILE* create_c_file(const char* furox_filepath, const char* filename)
{
    FRX_ASSERT(furox_filepath != NULL);

    FRX_ASSERT(filename != NULL);

    char c_filepath[strlen(furox_filepath) + strlen("/") + strlen(filename) + 1];

    const char* extension = strrchr(filename, '.');
    usize filename_len = extension - filename;

    if(extension == NULL)
        filename_len = strlen(filename);

    sprintf(c_filepath, "%s/%.*s.c", furox_filepath, (int)filename_len, filename);

    FILE* f = fopen(c_filepath, "w");
    if(f == NULL)
        return NULL;

    store_c_filepath(c_filepath);

    if(fprintf(f, "#include \"Furox.h\"\n#include \"%.*s.h\"\n", (int)filename_len, filename) < 0)
    {
        fclose(f);

        return NULL;
    }

    return f;
}

static FILE* create_header_file(const char* furox_filepath, const char* filename)
{
    FRX_ASSERT(furox_filepath != NULL);

    FRX_ASSERT(filename != NULL);

    char header_filepath[strlen(furox_filepath) + strlen("/") + strlen(filename) + 1];

    const char* extension = strrchr(filename, '.');
    usize filename_len = extension - filename;

    if(extension == NULL)
        filename_len = strlen(filename);

    sprintf(header_filepath, "%s/%.*s.h", furox_filepath, (int)filename_len, filename);

    FILE* f = fopen(header_filepath, "w");
    if(f == NULL)
        return NULL;

    for(usize i = 0; i < strlen(header_filepath); ++i)
        header_filepath[i] = isalnum(header_filepath[i]) ? toupper(header_filepath[i]) : '_';

    if(fprintf(f, "#ifndef %s\n#define %s\n\n", header_filepath, header_filepath) < 0)
    {
        fclose(f);
        
        return NULL;
    }

    return f;
}

FRX_NO_DISCARD b8 transpile_ast(const AST* root, const char* src_filepath)
{
    FRX_ASSERT(root != NULL);

    FRX_ASSERT(src_filepath != NULL);

    char furox_filepath[strlen(src_filepath) + strlen("furox-c/") + 1];

    const char* filename = strrchr(src_filepath, '/');
    if(filename == NULL)
        filename = src_filepath;

    if(create_furox_directory_from_source(src_filepath, furox_filepath))
        return FRX_TRUE;

    FILE* c_file = create_c_file(furox_filepath, filename);
    if(c_file == NULL)
        return FRX_TRUE;

    FILE* header_file = create_header_file(furox_filepath, filename);
    if(header_file == NULL)
    {
        fclose(c_file);
        
        return FRX_TRUE;
    }

    if(transpile_c(root, c_file) || transpile_header(root, header_file))
    {
        fclose(c_file);
        fclose(header_file);

        return FRX_TRUE;
    }

    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(header_file, "#endif\n");

    fclose(c_file);
    fclose(header_file);

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 generate_furox_main_c()
{
    FILE* f = fopen("furox-c/FuroxMain.c", "w");
    if(f == NULL)
        return FRX_TRUE;

    static const char* furox_main_c = "#include \"Furox.h\"\n\n"
                                      "int Main(void);\n\n"
                                      "int main(int argc, char** argv)\n"
                                      "{\n"
                                      "    return Main();\n"
                                      "}";

    fprintf(f, "%s", furox_main_c);

    fclose(f);

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 generate_furox_header()
{
    FILE* f = fopen("furox-c/Furox.h", "w");
    if(f == NULL)
        return FRX_TRUE;

    static const char* furox_header = "#ifndef FUROX_H\n"
                                      "#define FUROX_H\n\n"
                                      "#include <stddef.h>\n"
                                      "#include <stdint.h>\n\n"
                                      "typedef size_t usize;\n"
                                      "typedef int64_t isize;\n\n"
                                      "typedef int8_t i8;\n"
                                      "typedef int16_t i16;\n"
                                      "typedef int32_t i32;\n"
                                      "typedef int64_t i64;\n\n"
                                      "typedef uint8_t u8;\n"
                                      "typedef uint16_t u16;\n"
                                      "typedef uint32_t u32;\n"
                                      "typedef uint64_t u64;\n\n"
                                      "typedef u8 b8;\n"
                                      "typedef u16 b16;\n"
                                      "typedef u32 b32;\n"
                                      "typedef u64 b64;\n\n"
                                      "typedef float f32;\n"
                                      "typedef double f64;\n\n"
                                      "#endif";

    fprintf(f, "%s", furox_header);

    fclose(f);

    return FRX_FALSE;
}

FRX_NO_DISCARD b8 generate_executable(void)
{
    char command[4096];

    if(generate_furox_main_c())
        return FRX_TRUE;

    if(generate_furox_header())
        return FRX_TRUE;

    strcpy(command, "gcc furox-c/FuroxMain.c");

    for(usize i = 0; i < transpiler_info.c_filepaths_size; ++i)
    {
        strcat(command, " ");
        strcat(command, transpiler_info.c_filepaths[i]);
    }

    return system(command) == -1;
}
