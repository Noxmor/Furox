#include "transpiler.h"

#include <ctype.h>
#include <string.h>

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

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "'%c'", data->literal);

            break;
        }

        case FRX_AST_TYPE_STRING_LITERAL:
        {
            StringLiteralData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "\"%s\"", data->literal);

            break;
        }

        case FRX_AST_TYPE_VARIABLE_DECLARATION:
        {
            VariableData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s %s;\n", data->type, data->name);

            break;
        }

        case FRX_AST_TYPE_VARIABLE_DEFINITION:
        {
            VariableData* data = root->data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s %s = ", data->type, data->name);

            FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));

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

            if(root->children_size > 0)
            {
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ".");

                FRX_TRANSPILER_ABORT_ON_ERROR(transpile_c(&root->children[0], f));
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

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s ", data->return_type);

            FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);
            
            AST* parameter_list = &root->children[0];
            
            for(usize i = 0; i < parameter_list->children_size; ++i)
            {
                VariableData* parameter = parameter_list->children[i].data;

                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s %s", parameter->type, parameter->name);

                if(i != parameter_list->children_size - 1)
                    FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
            }
            
            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ")\n");

            return transpile_c(&root->children[1], f);
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

        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s ", data->return_type);
        
        FRX_TRANSPILER_ABORT_ON_ERROR(write_current_namespace(f));
        
        FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s(", data->name);
        
        AST* parameter_list = &root->children[0];

        for(usize i = 0; i < parameter_list->children_size; ++i)
        {
            VariableData* parameter = parameter_list->children[i].data;

            FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, "%s %s", parameter->type, parameter->name);

            if(i != parameter_list->children_size - 1)
                FRX_TRANSPILER_ABORT_ON_WRITE_ERROR(f, ", ");
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

static FILE* create_c_file(const char* src_filepath)
{
    FRX_ASSERT(src_filepath != NULL);

    char c_filepath[strlen(src_filepath) + strlen(".c") + 1];
    strcpy(c_filepath, src_filepath);
    char* extension = strrchr(c_filepath, '.');
    strcpy(++extension, "c");

    FILE* f = fopen(c_filepath, "w");
    if(f == NULL)
        return NULL;

    store_c_filepath(c_filepath);

    *extension = 'h';
    if(fprintf(f, "#include \"Furox.h\"\n#include \"%s\"\n", c_filepath) < 0)
    {
        fclose(f);

        return NULL;
    }

    return f;
}

static FILE* create_header_file(const char* src_filepath)
{
    FRX_ASSERT(src_filepath != NULL);

    char header_filepath[strlen(src_filepath) + strlen(".h") + 1];
    strcpy(header_filepath, src_filepath);
    char* extension = strrchr(header_filepath, '.');
    strcpy(++extension, "h");

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

    FILE* c_file = create_c_file(src_filepath);
    if(c_file == NULL)
        return FRX_TRUE;

    FILE* header_file = create_header_file(src_filepath);
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

FRX_NO_DISCARD b8 generate_executable(void)
{
    char command[4096];

    strcpy(command, "gcc FuroxMain.c");

    for(usize i = 0; i < transpiler_info.c_filepaths_size; ++i)
    {
        strcat(command, " ");
        strcat(command, transpiler_info.c_filepaths[i]);
    }

    return system(command) == -1;
}
