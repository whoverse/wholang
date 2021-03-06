#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "x86.h"

#define max(a, b) a > b ? a : b

unsigned int _log2(unsigned int x) {
    unsigned int ans = 0;

    while ( x >>= 1 ) {
        ans++;
    }

    return ans;
}

unsigned int _log10(unsigned int x) {
    unsigned int ans = 0;

    while ( x /= 10 ) {
        ans++;
    }

    return ans;
}

x86_arg_t* x86_reg(x86_register_t reg) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_reg;

    x86_reg_t* r = (x86_reg_t*) malloc(sizeof(x86_reg_t));
    r->reg = reg;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_deref_reg(x86_register_t reg) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_deref_reg;

    x86_deref_reg_t* r = (x86_deref_reg_t*) malloc(sizeof(x86_deref_reg_t));
    r->reg = reg;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_offset_reg(x86_register_t reg, int offset) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_reg_off;

    x86_reg_off_t* r = (x86_reg_off_t*) malloc(sizeof(x86_reg_off_t));
    r->reg = reg;
    r->amnt = offset;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_number(int number) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_number;

    x86_number_t* r = (x86_number_t*) malloc(sizeof(x86_number_t));
    r->number = number;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_ulong(unsigned long number) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_ulong;

    x86_ulong_t* r = (x86_ulong_t*) malloc(sizeof(x86_ulong_t));
    r->number = number;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_label(x86_label_t* label) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_label;

    x86_arg_label_t* r = (x86_arg_label_t*) malloc(sizeof(x86_arg_label_t));
    r->label = label;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_string(int ind) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_string;

    x86_string_t* r = (x86_string_t*) malloc(sizeof(x86_string_t));
    r->index = ind;
    arg->data = r;

    return arg;
}

x86_state_t* x86_init() {
    x86_state_t* cmp = (x86_state_t*) malloc(sizeof(x86_state_t));

    cmp->labels = INIT_LIST(x86_label_t);
    cmp->label_cnt = 0;
    cmp->label_alloc = MALLOC_CHUNK;

    cmp->variables = INIT_LIST(x86_variable_t);
    cmp->variable_cnt = 0;
    cmp->variable_alloc = MALLOC_CHUNK;

    cmp->strings = INIT_LIST(char); // wew lads
    cmp->string_cnt = 0;
    cmp->string_alloc = MALLOC_CHUNK;

    cmp->structs = INIT_LIST(x86_struct_t);
    cmp->struct_cnt = 0;
    cmp->struct_alloc = MALLOC_CHUNK;

    cmp->cur_label = NULL;
    cmp->curr_off = 0;
    cmp->curr_labels = 0;

    cmp->error = 0;

    return cmp;
}

void x86_error(x86_state_t* cmp, const char* error, ...) {
    va_list ap;

    printf("Error: ");
    va_start(ap, error);
    vfprintf(stdout, error, ap);
    va_end (ap);

    cmp->error = 1;
}

void x86_command_debug(x86_command_t* cmd) {
    const char* ops[] = {
        "PUSH",
        "PUSHB",
        "PUSHW",
        "PUSHL",

        "POP",
        "POPB",
        "POPW",
        "POPL",

        "ADD",
        "ADDB",
        "ADDW",
        "ADDL",
        "ADDSS",
        "ADDSD",

        "SUB",
        "SUBB",
        "SUBW",
        "SUBL",
        "SUBSS",
        "SUBSD",

        "MUL",
        "MULB",
        "MULW",
        "MULL",
        "MULSS",
        "MULSD",

        "IMUL",
        "IMULB",
        "IMULW",
        "IMULL",

        "LEA",
        "LEAB",
        "LEAW",
        "LEAL",

        "MOV",
        "MOVB",
        "MOVW",
        "MOVL",
        "MOVSS",
        "MOVSD",
        "MOVZBL",
        "MOVZWL",
        "MOVSBL",
        "MOVSWL",

        "MOVS",
        "MOVSB",
        "MOVSW",
        "MOVSL",
        "MOVZ",
        "MOVZB",
        "MOVZW",
        "MOVZL",

        "NEG",
        "NEGB",
        "NEGW",
        "NEGL",

        "SHL",
        "SHLB",
        "SHLW",
        "SHLL",

        "SHR",
        "SHRB",
        "SHRW",
        "SHRL",

        "AND",
        "ANDB",
        "ANDW",
        "ANDL",

        "OR",
        "ORB",
        "ORW",
        "ORL",

        "XOR",
        "XORB",
        "XORW",
        "XORL",

        "CVTSI2SS",
        "CVTSI2SD",
        "CVTSS2SI",
        "CVTSD2SI",

        "CMPL",
        "SETE",
        "SETLE",
        "SETGE",
        "SETL",
        "SETG",

        "JMP",
        "JE",
        "JNE",
        "JG",
        "JGE",
        "JL",
        "JLE",

        "RET",
        "LEAVE",
        "CALL"
    };

    const char* regs[] = {
        "EAX",
        "ECX",
        "EDX",
        "EBX",
        "ESP",
        "EBP",
        "ESI",
        "EDI",

        "XMM0",
        "XMM1",
        "XMM2",
        "XMM3",
        "XMM4",
        "XMM5",
        "XMM6",
        "XMM7",

        "AX",
        "BX",
        "CX",
        "DX",

        "AH",
        "AL",
        "BH",
        "BL",
        "CH",
        "CL",
        "DH",
        "DL"
    };

    printf("\t%s ", ops[cmd->inst]);

    if ( cmd->arg1 != NULL ) {
        if ( cmd->arg1->type == arg_reg ) {
            printf("%%%s", regs[((x86_reg_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_deref_reg ) {
            printf("(%%%s)", regs[((x86_deref_reg_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_deref_off ) {
            int amnt = ((x86_deref_off_t*) cmd->arg1->data)->amnt;
            printf("%d(%%%s)", amnt, regs[((x86_deref_off_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_reg_off ) {
            printf("%d(%%%s)", ((x86_reg_off_t*) cmd->arg1->data)->amnt, regs[((x86_reg_off_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_number ) {
            printf("$%d", ((x86_number_t*) cmd->arg1->data)->number);
        }
        else if ( cmd->arg1->type == arg_label ) {
            printf("%s", ((x86_arg_label_t*) cmd->arg1->data)->label->name);
        }
        else if ( cmd->arg1->type == arg_ulong ) {
            printf("$%lu", ((x86_ulong_t*) cmd->arg1->data)->number);
        }
        else if ( cmd->arg1->type == arg_string ) {
            printf("$.LC%d", ((x86_string_t*) cmd->arg1->data)->index);
        }

        if ( cmd->arg2 != NULL ) {
            printf(", ");

            if ( cmd->arg2->type == arg_reg ) {
                printf("%%%s", regs[((x86_reg_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_deref_reg ) {
                printf("(%%%s)", regs[((x86_deref_reg_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_deref_off ) {
                int amnt = ((x86_deref_off_t*) cmd->arg2->data)->amnt;
                printf("%d(%%%s)", amnt, regs[((x86_deref_off_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_reg_off ) {
                printf("%d(%%%s)", ((x86_reg_off_t*) cmd->arg2->data)->amnt, regs[((x86_reg_off_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_number ) {
                printf("$%d", ((x86_number_t*) cmd->arg2->data)->number);
            }
            else if ( cmd->arg2->type == arg_label ) {
                printf("%s", ((x86_arg_label_t*) cmd->arg2->data)->label->name);
            }
            else if ( cmd->arg2->type == arg_ulong ) {
                printf("$%lu", ((x86_ulong_t*) cmd->arg2->data)->number);
            }
            else if ( cmd->arg2->type == arg_string ) {
                printf("$.LC%d", ((x86_string_t*) cmd->arg2->data)->index);
            }
        }
    }

    printf("\n");
}

void x86_label_debug(x86_label_t* label) {
    if ( label->global ) {
        printf(".global %s\n", label->name);
    }
    printf("%s:\n", label->name);


    for ( int i = 0; i < label->cmd_cnt; i++ ) {
        x86_command_debug(label->cmds[i]);
    }
}

void x86_clean(x86_state_t* cmp) {
    if ( cmp->labels != NULL ) {
        for ( int i = 0; i < cmp->label_cnt; i++ ) {
            x86_label_t* label = cmp->labels[i];

            if ( label == NULL ) continue;

            for ( int j = 0; j < label->cmd_cnt; j++ ) {
                x86_command_t* cmd = label->cmds[j];
                if ( cmd->arg1 ) {
                    free(cmd->arg1->data);
                    free(cmd->arg1);
                }
                if ( cmd->arg2 ) {
                    free(cmd->arg2->data);
                    free(cmd->arg2);
                }
                free(cmd);
            }
            free(label->cmds);
            free((void*) label->name);
            free(label);
        }
        free(cmp->labels);
    }

    if ( cmp->cur_label != NULL ) {
        for ( int i = 0; i < cmp->cur_label->cmd_cnt; i++ ) {
            x86_command_t* cmd = cmp->cur_label->cmds[i];
            if ( cmd->arg1 ) {
                free(cmd->arg1->data);
                free(cmd->arg1);
            }
            if ( cmd->arg2 ) {
                free(cmd->arg2->data);
                free(cmd->arg2);
            }
            free(cmd);
        }
        free(cmp->cur_label->cmds);
        free((void*) cmp->cur_label->name);
        free(cmp->cur_label);
    }

    if ( cmp->variables != NULL ) {
        for ( int i = 0; i < cmp->variable_cnt; i++ ) {
            x86_variable_t* var = cmp->variables[i];

            if ( var == NULL ) continue;

            if ( var->type != NULL ) free(var->type);

            if ( var->name != NULL ) free((void*) var->name);

            free(var);
        }

        free(cmp->variables);
    }

    if ( cmp->structs != NULL ) {
        for ( int j = 0; j < cmp->struct_cnt; j++ ) {
            x86_struct_t* cmp_strc = cmp->structs[j];

            if ( cmp_strc != NULL ) {
                if ( cmp_strc->args != NULL ) {
                    for ( int i = 0; i < cmp_strc->arg_cnt; i++ ) {
                        if ( cmp_strc->args[i] ) {
                            if ( cmp_strc->args[i]->type ) free(cmp_strc->args[i]->type);
                            if ( cmp_strc->args[i]->name ) free(cmp_strc->args[i]->name);

                            free(cmp_strc->args[i]);
                        }
                    }

                    free(cmp_strc->args);
                }

                if ( cmp_strc->name ) free(cmp_strc->name);

                free(cmp_strc);
            }
        }


        free(cmp->structs);
    }

    if ( cmp->strings != NULL ) {
        free(cmp->strings);
    }

    free(cmp);
}

int x86_find_label(x86_state_t* cmp, const char* label) {
    for ( int i = 0; i < cmp->label_cnt; i++ ) {
        if ( cmp->labels[i] != NULL && strcmp(cmp->labels[i]->name, label) == 0 ) {
            return i;
        }
    }

    return -1;
}

x86_type_t* x86_type_string(x86_state_t* cmp, const char* str) {
    if ( str == NULL || cmp == NULL ) {
        return NULL;
    }

    if ( strstr(str, "*") != NULL ) {
        int real = strstr(str, "*") - str;

        char* tmp = malloc((real + 1) * sizeof(char));

        if ( tmp == NULL ) {
            return NULL;
        }

        memcpy(tmp, str, real);
        tmp[real] = '\0';

        x86_type_t* type = x86_type_string(cmp, tmp);
        type->deref_cnt = strlen(str) - real;

        free(tmp);
        return type;
    }

    if ( strcmp(str, "uint8_t") == 0 || strcmp(str, "char") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 1;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "int8_t") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 1;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "uint16_t") == 0 || strcmp(str, "short") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 2;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "int16_t") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 2;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "uint32_t") == 0 || strcmp(str, "int") == 0 || strcmp(str, "long") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 4;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "int32_t") == 0 || strcmp(str, "uint") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 4;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "float") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_float;
        type->size = 4;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "double") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_double;
        type->size = 8;
        type->deref_cnt = 0;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else if ( strcmp(str, "str") == 0 ) {
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->prim = prim_uint;
        type->size = 1;
        type->deref_cnt = 1;
        type->used = false;
        type->is_const = false;
        type->is_static = false;
        type->is_struct = false;
        type->struct_ind = 0;
        return type;
    }
    else {
        for ( int i = 0; i < cmp->parser->typedef_len; i++ ) {
            typedef_t* type = cmp->parser->typedefs[i];

            if ( strcmp(type->name, (char*) type) == 0 ) {
                return x86_type(cmp, type->type);
            }
        }

        return NULL;
    }
}

void x86_compile_struct(x86_state_t* cmp, struct_t* strc) {
    x86_struct_t* cmp_strc = (x86_struct_t*) malloc(sizeof(x86_struct_t));

    if ( cmp_strc == NULL ) {
        goto error;
    }

    if ( strc->name ) {
        cmp_strc->name = strdup(strc->name);

        if ( cmp_strc->name == NULL ) {
            goto error;
        }
    }

    cmp_strc->arg_cnt = 0;
    cmp_strc->arg_alloc = strc->len * (sizeof(x86_struct_arg_t*));
    cmp_strc->args = malloc(cmp_strc->arg_alloc);

    cmp_strc->size = 0;

    for ( int i = strc->len - 1; i >= 0; i-- ) {
        x86_struct_arg_t* arg = (x86_struct_arg_t*) malloc(sizeof(x86_struct_arg_t));
        arg->type = x86_type(cmp, strc->args[i]->type);
        arg->type->used = true;
        arg->offset = cmp_strc->size;
        cmp_strc->size += arg->type->size;
        arg->name = strdup(strc->args[i]->name);

        ADD_ARG(cmp_strc->args, cmp_strc->arg_cnt, cmp_strc->arg_alloc, arg, {})
    }

    cmp_strc->size = (cmp_strc->size + 15) & ~15;


    ADD_ARG(cmp->structs, cmp->struct_cnt, cmp->struct_alloc, cmp_strc, {})

    return;
    error:

    if ( cmp_strc != NULL ) {
        if ( cmp_strc->args != NULL ) {
            for ( int i = 0; i < cmp_strc->arg_cnt; i++ ) {
                if ( cmp_strc->args[i] ) {
                    if ( cmp_strc->args[i]->type ) free(cmp_strc->args[i]->type);
                    if ( cmp_strc->args[i]->name ) free(cmp_strc->args[i]->name);

                    free(cmp_strc->args[i]);
                }
            }

            free(cmp_strc->args);
        }

        free(cmp_strc);
    }

    cmp->error = true;
    return;
}

x86_type_t* x86_type(x86_state_t* cmp, type_t* type) {
    if ( cmp == NULL || type == NULL ) {
        return NULL;
    }

    if ( type->is_struct ) {
        for ( int i = 0; i < cmp->struct_cnt; i++ ) {
            if ( strcmp(cmp->structs[i]->name, type->name) == 0 ) { // TODO - Better
                x86_type_t* xtype = (x86_type_t*) malloc(sizeof(x86_type_t));
                xtype->prim = prim_struct;
                xtype->size = cmp->structs[i]->size;
                xtype->deref_cnt = type->deref_cnt;
                xtype->used = false;
                xtype->is_const = false;
                xtype->is_static = false;
                xtype->is_struct = true;
                xtype->struct_ind = i;

                return xtype;
            }
        }

        return NULL;
    }
    else {
        x86_type_t* xtype = x86_type_string(cmp, type->name);

        if ( xtype->deref_cnt == 0 ) {
            xtype->deref_cnt = type->deref_cnt;
        }

        xtype->is_static = type->is_static;
        xtype->is_const = type->is_const;

        return xtype;
    }
}

bool x86_type_equ(x86_state_t* cmp, x86_type_t* type1, x86_type_t* type2) {
    if ( !type1->is_struct && !type2->is_struct ) {
        return type1->prim == type2->prim && type1->deref_cnt == type2->deref_cnt &&
               type1->size <= type2->size && type1->is_const == type2->is_const;
    }
    else {
        if ( type1->is_struct && type2->is_struct ) {
            x86_struct_t* struct1 = cmp->structs[type1->struct_ind];
            x86_struct_t* struct2 = cmp->structs[type2->struct_ind];

            if ( struct1 == NULL || struct2 == NULL ) return false;
            if ( struct1->arg_cnt != struct2->arg_cnt ) return false;

            for ( int i = 0; i < struct1->arg_cnt; i++ ) {
                if ( !x86_type_equ(cmp, struct1->args[i]->type, struct2->args[i]->type) ) {
                    return false;
                }

                if ( struct1->args[i]->name != NULL ) {
                    if ( struct2->args[i]->name != NULL ) {
                        if ( strcmp(struct1->args[i]->name, struct2->args[i]->name) != 0 ) {
                            return false;
                        }
                    }
                    else {
                        return false;
                    }
                }
                else {
                    if ( struct2->args[i]->name != NULL ) {
                        return false;
                    }
                }
            }

            return true;
        }
        else {
            return false;
        }
    }
}

x86_variable_t* x86_find_variable(x86_state_t* cmp, const char* name) {
    for ( int i = 0; i < cmp->variable_cnt; i++ ) {
        if ( strcmp(cmp->variables[i]->name, name) == 0 ) {
            return cmp->variables[i];
        }
    }

    return NULL;
}

x86_type_t* x86_compile_fncall(x86_state_t* cmp, expr_call_t* call) {
    x86_label_t* label = cmp->cur_label;
    x86_label_t* c_label = NULL;

    for ( int i = 0; i < cmp->label_cnt; i++ ) {
        if ( strcmp(cmp->labels[i]->name, call->name) == 0 ) {
            c_label = cmp->labels[i];
            break;
        }
    }

    if ( c_label == NULL ) {
        x86_error(cmp, "Unkown function \"%s\"", call->name);
        goto error;
    }

    if ( c_label->fn ) {
        function_t* fn = c_label->fn;

        if ( call->len != fn->len ) {
            x86_error(cmp, "Function argument length mismatch\n");
            goto error;
        }

        int add = 0;

        for ( int i = call->len - 1; i >= 0; i-- ) {
            x86_type_t* want = x86_type(cmp, fn->args[i]->type);

            if ( want->prim == prim_struct ) {
                x86_type_t* type = x86_compile_expression(cmp, call->args[i]);

                if ( want == NULL ) {
                    free(want);
                    if ( !type->used ) free(type);

                    x86_error(cmp, "Invalid argument type \"%s\"\n", fn->args[i]->type->name);
                    goto error;
                }

                _x86_label_remove(label, label->cmd_cnt - 1);
                _x86_label_remove(label, label->cmd_cnt - 1);

                if ( !x86_type_equ(cmp, type, want) ) {
                    free(want);
                    if ( !type->used ) free(type);

                    x86_error(cmp, "Function argument type mismatch");
                    goto error;
                }

                x86_struct_t* struc = cmp->structs[type->struct_ind];

                for ( int i = 0; i < struc->arg_cnt; i++ ) {
                    int off = cmp->structs[type->struct_ind]->args[i]->offset + type->offset;
                    x86_type_t* ret = cmp->structs[type->struct_ind]->args[i]->type;
                    ASM(MOV, x86_offset_reg(EBP, off), x86_reg(EAX))
                    ASM(PUSH, x86_reg(EAX), NULL)
                    add += cmp->structs[type->struct_ind]->args[i]->type->size;
                }
            }
            else {
                x86_type_t* type = x86_compile_expression(cmp, call->args[i]);

                if ( want == NULL ) {
                    free(want);
                    if ( !type->used ) free(type);

                    x86_error(cmp, "Invalid argument type \"%s\"\n", fn->args[i]->type->name);
                    goto error;
                }

                if ( !x86_type_equ(cmp, type, want) ) {
                    free(want);
                    if ( !type->used ) free(type);

                    x86_error(cmp, "Function argument type mismatch");
                    goto error;
                }

                add += type->size;
                free(type);
            }

            if ( !want->used ) free(want);
        }

        ASM(CALL, x86_label(c_label), NULL)
        ASM(ADD, x86_number(add), x86_reg(ESP))
        ASM(PUSH, x86_reg(EAX), NULL)

        return x86_type(cmp, fn->ret_type);
    }

    error:
    // FUK
    return NULL;
}

x86_type_t* x86_compile_expression(x86_state_t* cmp, expr_t* expr) {
    x86_label_t* label = cmp->cur_label;

    if ( expr->type == expr_prefix ) {
        expr_prefix_t* prfx = expr->value;
        x86_type_t* type = x86_compile_expression(cmp, prfx->lhs);

        if ( type == NULL ) {
            goto error;
        }

        ASM(POP, x86_reg(EAX), NULL)

        if ( prfx->op.token_val == NEG_OP ) {
            ASM(NEGL, x86_reg(EAX), NULL)
        }
        else if ( prfx->op.token_val == OPP_OP ) {
            // THIS IS HARD
        }
        else if ( prfx->op.token_val == DEREF_OP ) {
            if ( type->deref_cnt == 1 ) {
                if ( type->size < 4 ) {
                    if ( type->prim == prim_uint ) {
                        ASM(MOVZ + 1 + _log2(type->size), x86_deref_reg(EAX), x86_reg(EAX))
                    }
                    else {
                        ASM(MOVS + 1 + _log2(type->size), x86_deref_reg(EAX), x86_reg(EAX))
                    }
                }
                else {
                    ASM(MOVL, x86_deref_reg(EAX), x86_reg(EAX))
                }
            }
            else {
                if ( type->deref_cnt > 1 ) {
                    ASM(MOVL, x86_deref_reg(EAX), x86_reg(EAX))
                }
                else {
                    x86_error(cmp, "Unable to derefence value.\n");
                    goto error;
                }
            }

            type->deref_cnt -= 1;
        }
        else if ( prfx->op.token_val == ADDR_OP ) {
            if ( prfx->lhs->type == expr_name ) {
                x86_variable_t* var = x86_find_variable(cmp, prfx->lhs->value);

                if ( var == NULL ) {
                    x86_error(cmp, "Unable to find variable \"%s\"\n", prfx->lhs->value);
                    goto error;
                }

                ASM(LEAL, x86_offset_reg(EBP, var->offset), x86_reg(EAX))
            }
            else {
                x86_error(cmp, "Unable to convert non-name to pointer.\n");
                goto error;
            }

            type->deref_cnt += 1;
        }
        else {
            x86_error(cmp, "Unable to compile prefix operator.\n");
        }

        ASM(PUSH, x86_reg(EAX), NULL)
        return type;
    }
    else if ( expr->type == expr_infix ) {
        expr_infix_t* infx = expr->value;

        // if ( infx->rhs->type == expr_expr || infx->rhs->type == expr_infix ) {
        //     expr_t* tmp = infx->rhs;
        //     infx->rhs = infx->lhs;
        //     infx->lhs = tmp;
        // }

        if ( infx->op.token_val == MBR_BIN_OP || infx->op.token_val == DMBR_BIN_OP ) {
            x86_type_t* type1 = x86_compile_expression(cmp, infx->lhs);

            if ( type1 == NULL ) {
                goto error;
            }

            if ( infx->rhs->type != expr_name ) {
                x86_error(cmp, "Must use names on the right hand of struct dereferences.\n");
                goto error;
            }

            if ( type1->prim != prim_struct ) {
                x86_error(cmp, "Must dereference struct values.\n");
                goto error;
            }

            int off = -1;
            x86_type_t* ret;

            for ( int i = 0; i < cmp->structs[type1->struct_ind]->arg_cnt; i++ ) {
                if ( strcmp(cmp->structs[type1->struct_ind]->args[i]->name, infx->rhs->value) == 0 ) {
                    off = cmp->structs[type1->struct_ind]->args[i]->offset;
                    ret = cmp->structs[type1->struct_ind]->args[i]->type;

                    break;
                }
            }

            if ( off == -1 ) {
                x86_error(cmp, "Invalid struct member %s\n", infx->rhs->value, infx->rhs->value);
                goto error;
            }

            off = off + type1->offset;

            // yuck out of ten
            // THIS IS EW
            ret->offset = off; // s U C H A B AAAAAAA D I D E A
            // BE GROSSED OUT ^^^

            _x86_label_remove(label, label->cmd_cnt - 1);
            _x86_label_remove(label, label->cmd_cnt - 1);

            ASM(MOV, x86_offset_reg(EBP, off), x86_reg(EAX))

            if ( infx->op.token_val == DMBR_BIN_OP ) {
                if ( ret->deref_cnt == 0 ) {
                    x86_error(cmp, "Can't dereference nonpointer\n");
                    goto error;
                }

                ret->deref_cnt -= 1;
                ASM(MOVL, x86_deref_reg(EAX), x86_reg(EAX))
            }

            ASM(PUSH, x86_reg(EAX), NULL)

            return ret;
        }

        x86_type_t* type2 = x86_compile_expression(cmp, infx->rhs);

        if ( type2 == NULL ) {
            goto error;
        }

        x86_type_t* type1 = x86_compile_expression(cmp, infx->lhs);

        if ( type1 == NULL) {
            if ( !type2->used ) free(type2);
            goto error;
        }

        if ( type1->prim == prim_float ) {
            ASM(POP, x86_reg(XMM0), NULL)

            if ( type2->prim == prim_int ) {
                ASM(POP, x86_reg(EBX), NULL)

                if ( type2->size != 4 ) {
                    if ( type2->size == 1 ) {
                        ASM(MOVZBL, x86_reg(EBX), x86_reg(EBX))
                    }
                    else if ( type2->size == 2 ) {
                        ASM(MOVZWL, x86_reg(EBX), x86_reg(EBX))
                    }
                    else {
                        x86_error(cmp, "Invalid integer size %d\n", type2->size);
                        goto error;
                    }
                }

                ASM(CVTSI2SS, x86_reg(EBX), x86_reg(XMM1))
            }
            else if ( type2->prim == prim_uint ) {
                ASM(POP, x86_reg(EBX), NULL)

                if ( type2->size != 4 ) {
                    if ( type2->size == 1 ) {
                        ASM(MOVSBL, x86_reg(EBX), x86_reg(EBX))
                    }
                    else if ( type2->size == 2 ) {
                        ASM(MOVSWL, x86_reg(EBX), x86_reg(EBX))
                    }
                    else {
                        x86_error(cmp, "Invalid integer size %d\n", type2->size);
                        goto error;
                    }
                }
            }
            else if ( type2->prim == prim_float ) {
                ASM(POP, x86_reg(XMM1), NULL)
            }
            else {
                // ???
            }
        }
        else if ( type2->prim == prim_float ) {
            ASM(POP, x86_reg(XMM1), NULL)

            if ( type1->prim == prim_int ) {
                ASM(POP, x86_reg(EAX), NULL)

                if ( type1->size != 4 ) {
                    if ( type1->size == 1 ) {
                        ASM(MOVZBL, x86_reg(EAX), x86_reg(EAX))
                    }
                    else if ( type1->size == 2 ) {
                        ASM(MOVZWL, x86_reg(EAX), x86_reg(EAX))
                    }
                    else {
                        x86_error(cmp, "Invalid integer size %d\n", type1->size);
                        goto error;
                    }
                }

                ASM(CVTSI2SS, x86_reg(EAX), x86_reg(XMM0))
            }
            else if ( type1->prim == prim_uint ) {
                ASM(POP, x86_reg(EAX), NULL)

                if ( type1->size != 4 ) {
                    if ( type1->size == 1 ) {
                        ASM(MOVSBL, x86_reg(EAX), x86_reg(EAX))
                    }
                    else if ( type1->size == 2 ) {
                        ASM(MOVSWL, x86_reg(EAX), x86_reg(EAX))
                    }
                    else {
                        x86_error(cmp, "Invalid integer size %d\n", type1->size);
                        goto error;
                    }
                }
            }
            else {
                // ???
            }
        }
        else {
            ASM(POP, x86_reg(EAX), NULL)
            ASM(POP, x86_reg(EBX), NULL)
        }

        if ( infx->op.token_val == ADD_BIN_OP ) {
            ASM(ADD + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == SUB_BIN_OP ) {
            ASM(SUB + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == MUL_BIN_OP ) {
            // if ( type1->prim == prim_int || type2->prim == prim_int ) {
                ASM(IMUL + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
            // }
            // else {
            //     ASM(MUL + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
            // }
        }
        else if ( infx->op.token_val == DIV_BIN_OP ) {
            // TO DO
        }
        else if ( infx->op.token_val == SHL_BIN_OP ) {
            if ( (type1->prim != prim_uint && type1->prim != prim_int && type1->deref_cnt == 0) ||
                 (type2->prim != prim_uint && type2->prim != prim_int && type2->deref_cnt == 0)) {
                x86_error(cmp, "Bitwise operations require integers or pointers.\n");
                goto error;
            }
            ASM(SHL + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == SHR_BIN_OP ) {
            if ( (type1->prim != prim_uint && type1->prim != prim_int && type1->deref_cnt == 0) ||
                 (type2->prim != prim_uint && type2->prim != prim_int && type2->deref_cnt == 0)) {
                x86_error(cmp, "Bitwise operations require integers or pointers.\n");
                goto error;
            }
            ASM(SHR + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == AND_BIN_OP ) {
            if ( (type1->prim != prim_uint && type1->prim != prim_int && type1->deref_cnt == 0) ||
                 (type2->prim != prim_uint && type2->prim != prim_int && type2->deref_cnt == 0)) {
                x86_error(cmp, "Bitwise operations require integers or pointers.\n");
                goto error;
            }
            ASM(AND + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == OR_BIN_OP ) {
            if ( (type1->prim != prim_uint && type1->prim != prim_int && type1->deref_cnt == 0) ||
                 (type2->prim != prim_uint && type2->prim != prim_int && type2->deref_cnt == 0)) {
                x86_error(cmp, "Bitwise operations require integers or pointers.\n");
                goto error;
            }
            ASM(OR + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == XOR_BIN_OP ) {
            if ( (type1->prim != prim_uint && type1->prim != prim_int && type1->deref_cnt == 0) ||
                 (type2->prim != prim_uint && type2->prim != prim_int && type2->deref_cnt == 0)) {
                x86_error(cmp, "Bitwise operations require integers or pointers.\n");
                goto error;
            }
            ASM(XOR + 1 + _log2(max(type1->size, type2->size)), x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == ET_BIN_OP ) {
            ASM(CMPL, x86_reg(EBX), x86_reg(EAX))
            ASM(SETE, x86_reg(AL), NULL)
            ASM(MOVZBL, x86_reg(AL), x86_reg(EAX))
        }
        else if ( infx->op.token_val == GT_BIN_OP ) {
            ASM(CMPL, x86_reg(EBX), x86_reg(EAX))
            ASM(SETG, x86_reg(AL), NULL)
            ASM(MOVZBL, x86_reg(AL), x86_reg(EAX))
        }
        else if ( infx->op.token_val == GTE_BIN_OP ) {
            ASM(CMPL, x86_reg(EBX), x86_reg(EAX))
            ASM(SETGE, x86_reg(AL), NULL)
            ASM(MOVZBL, x86_reg(AL), x86_reg(EAX))
        }
        else if ( infx->op.token_val == LT_BIN_OP ) {
            ASM(CMPL, x86_reg(EBX), x86_reg(EAX))
            ASM(SETL, x86_reg(AL), NULL)
            ASM(MOVZBL, x86_reg(AL), x86_reg(EAX))
        }
        else if ( infx->op.token_val == LTE_BIN_OP ) {
            ASM(CMPL, x86_reg(EBX), x86_reg(EAX))
            ASM(SETLE, x86_reg(AL), NULL)
            ASM(MOVZBL, x86_reg(AL), x86_reg(EAX))
        }

        ASM(PUSH, x86_reg(EAX), NULL)

        if ( type1->prim == prim_struct || type2->prim == prim_struct) {
            free(type1);
            free(type2);

            x86_error(cmp, "Unable to do arithmatic on structs");
            goto error;
        }
        else if ( (type1->prim == prim_void && type1->deref_cnt > 0) ||
                  (type2->prim == prim_void && type2->deref_cnt > 0) ) {
            if ( type1->prim == prim_void ) {
                if ( !type2->used ) free(type2);
                return type1;
            }
            else {
                if ( !type1->used ) free(type1);
                return type2;
            }
        }
        else if ( type1->prim == prim_uint || type2->prim == prim_uint ||
                  type1->prim == prim_int || type2->prim == prim_int) {
            if ( type1->prim == prim_int ) {
                if ( !type2->used ) free(type2);
                return type1;
            }
            else {
                if ( !type1->used ) free(type1);
                return type2;
            }
        }
        else {
            free(type1);
            free(type2);

            x86_error(cmp, "Unable to resolve type.\n");
            goto error;
        }
    }
    else if ( expr->type == expr_int ) {
        ASM(PUSH, x86_number(*(int*)expr->value), NULL)
        return x86_type_string(cmp, "int");
    }
    else if ( expr->type == expr_name ) {
        x86_variable_t* var = x86_find_variable(cmp, expr->value);

        if ( var == NULL ) {
            x86_error(cmp, "Unable to resolve variable \"%s\"\n", expr->value);
            goto error;
        }

        ASM(MOV, x86_offset_reg(EBP, var->offset), x86_reg(EAX))
        ASM(PUSH, x86_reg(EAX), NULL)

        var->type->offset = var->offset;
        var->type->used = true;
        
        return var->type;
    }
    else if ( expr->type == expr_expr ) {
        return x86_compile_expression(cmp, expr->value);
    }
    else if ( expr->type == expr_call ) {
        return x86_compile_fncall(cmp, expr->value);
    }
    else if ( expr->type == expr_double ) {
        ASM(PUSH, x86_ulong(*(long*)expr->value), NULL)
        return x86_type_string(cmp, "double");
    }
    else if ( expr->type == expr_string ) {
        ASM(PUSH, x86_string(x86_resolve_string(cmp, expr->value)), NULL)
        x86_type_t* type = x86_type_string(cmp, "str");
        type->is_const = true;
        return type;
    }
    else if ( expr->type == expr_cast ) {
        expr_cast_t* prfx = expr->value;
        x86_type_t* type = x86_compile_expression(cmp, prfx->rhs);
        ASM(POP, x86_reg(EAX), NULL)

        x86_type_t* new = x86_type(cmp, prfx->type);

        if ( new == NULL ) {
            if ( !type->used ) free(type);

            x86_error(cmp, "Unable to resolve type \"%s\"\n", prfx->type->name);
            goto error;
        }

        if ( new->prim == type->prim && new->size == type->size && new->deref_cnt == type->deref_cnt ) {
            ASM(PUSH, x86_reg(EAX), NULL)
            free(new);
            return type;
        }

        // TODO - handle in memory typecasts lmao
        ASM(PUSH, x86_reg(EAX), NULL)

        if ( !type->used ) free(type);

        return new;
    }

    error:
    return NULL;
}

void x86_compile_statement(x86_state_t* cmp, stmt_t* stmt) {
    x86_label_t* label = cmp->cur_label;

    if ( stmt->type == vardec_stmt ) {
        vardec_stmt_t* dec = stmt->data;

        if ( dec->name == NULL ) {
            goto error;
        }

        if ( x86_find_variable(cmp, dec->name) != NULL ) {
            return;
        }

        x86_variable_t* var = (x86_variable_t*) malloc(sizeof(x86_variable_t));

        if ( dec->type ) {
            var->type = x86_type(cmp, dec->type);

            if ( var->type == NULL ) {
                free(var);

                x86_error(cmp, "Unable to resolve type \"%s\"\n", dec->type->name);
                goto error;
            }

            var->type->used = true;

            cmp->curr_off -= var->type->deref_cnt > 0 ? 4 : var->type->size;

            if ( var->type->is_struct ) {
                cmp->curr_off = -((-cmp->curr_off + 15) & ~15);
            }

            var->name = strdup(dec->name);
            var->size = var->type->deref_cnt > 0 ? 4 : var->type->size;

            if ( var->name == NULL ) {
                free(var->type);
                free(var);

                x86_error(cmp, "Unable to resolve enough space for a variable name.\n");
                goto error;
            }

            var->offset = cmp->curr_off;

            if ( dec->value != NULL ) {
                x86_type_t* type = x86_compile_expression(cmp, dec->value);

                if ( type == NULL ) {
                    free((void*) var->name);
                    free(var->type);
                    free(var);

                    x86_error(cmp, "Unable to resolve type \"%s\"\n", dec->type->name);
                    goto error;
                }

                if ( !x86_type_equ(cmp, type, var->type) ) {
                    free(var->type);
                    free((void*) var->name);
                    free(var);
                    if ( !type->used ) free(type);

                    x86_error(cmp, "Type value mismatch\n");
                    goto error;
                }

                if ( !type->used ) free(type);

                ASM(POP, x86_reg(EAX), NULL)
                ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, var->offset))
            }
        }
        else {
            if ( dec->value == NULL ) {
                free(var);

                x86_error(cmp, "Unable to resolve type.");
                goto error;
            }

            x86_type_t* type = x86_compile_expression(cmp, dec->value);

            if ( type == NULL ) {
                free((void*) var->name);
                free(var);

                x86_error(cmp, "Unable to resolve type \"%s\"\n", dec->type->name);
                goto error;
            }

            var->type = type;

            cmp->curr_off -= var->type->deref_cnt > 0 ? 4 : var->type->size;

            var->name = strdup(dec->name);
            var->size = var->type->deref_cnt > 0 ? 4 : var->type->size;

            if ( var->name == NULL ) {
                free(var);

                x86_error(cmp, "Unable to resolve enough space for a variable name.\n");
                goto error;
            }

            var->offset = cmp->curr_off;

            ASM(POP, x86_reg(EAX), NULL)
            ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, var->offset))
        }


        ADD_ARG(cmp->variables, cmp->variable_cnt, cmp->variable_alloc, var, {
            free(var);
        })
    }
    else if ( stmt->type == ret_stmt ) {
        x86_type_t* type = x86_compile_expression(cmp, (expr_t*) stmt->data);

        if ( type == NULL ) {
            goto error;
        }

        ASM(POP, x86_reg(EAX), NULL)
        ASM(LEAVE, NULL, NULL)
        ASM(RET, NULL, NULL)

        if ( !type->used ) {
            free(type);
        }
    }
    else if ( stmt->type == call_stmt ) {
        free(x86_compile_fncall(cmp, (expr_call_t*) stmt->data)); // kill me
    }
    else if ( stmt->type == varset_stmt ) {
        varset_stmt_t* varset = stmt->data;
        expr_t* lhs = varset->lhs;

        if ( lhs->type == expr_name ) {
            char* name = lhs->value;

            x86_variable_t* var = x86_find_variable(cmp, name);

            if ( var == NULL ) {
                x86_error(cmp, "Unable to resolve variable \"%s\"\n", name);
                goto error;
            }

            if ( var->type->is_static || var->type->is_const ) {
                x86_error(cmp, "Unable to redeclare a static/const type.\n");
                goto error;
            }

            x86_type_t* type = x86_compile_expression(cmp, varset->value);

            if ( type == NULL ) {
                goto error;
            }

            if ( !x86_type_equ(cmp, var->type, type) ) {
                free(type);

                x86_error(cmp, "Type mismatch\n");
                goto error;
            }

            ASM(POP, x86_reg(EAX), NULL)

            if ( varset->mod == 0 ) {
                ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, var->offset))
            }
            else if ( varset->mod == '+' ) {
                ASM(MOV, x86_offset_reg(EBP, var->offset), x86_reg(EBX))
                ASM(ADD + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, var->offset))
            }
            else if ( varset->mod == '-' ) {
                ASM(MOV, x86_offset_reg(EBP, var->offset), x86_reg(EBX))
                ASM(SUB + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, var->offset))
            }
            else if ( varset->mod == '*' ) {
                ASM(MOV, x86_offset_reg(EBP, var->offset), x86_reg(EBX))
                ASM(IMUL + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, var->offset))
            }

            if ( !type->used ) free(type);
        }
        else if ( lhs->type == expr_infix) {
            expr_infix_t* infx = lhs->value;
            x86_type_t* type = x86_compile_expression(cmp, lhs);

            if ( type == NULL ) {
                goto error;
            }

            if ( type->is_static || type->is_const ) {
                x86_error(cmp, "Unable to redeclare a static/const type.\n");
                goto error;
            }

            _x86_label_remove(label, label->cmd_cnt - 1);
            _x86_label_remove(label, label->cmd_cnt - 1);

            x86_type_t* type1 = x86_compile_expression(cmp, varset->value);

            if ( !x86_type_equ(cmp, type, type1) ) {
                if ( !type->used ) free(type);
                if ( !type1->used ) free(type1);

                x86_error(cmp, "Type mismatch\n");
                goto error;
            }

            ASM(POP, x86_reg(EBX), NULL)

            // TO DO - DEREFERENCE

            if ( varset->mod == 0 ) {
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, type->offset))
            }
            else if ( varset->mod == '+' ) {
                ASM(MOV, x86_offset_reg(EBP, type->offset), x86_reg(EBX))
                ASM(ADD + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, type->offset))
            }
            else if ( varset->mod == '-' ) {
                ASM(MOV, x86_offset_reg(EBP, type->offset), x86_reg(EBX))
                ASM(SUB + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, type->offset))
            }
            else if ( varset->mod == '*' ) {
                ASM(MOV, x86_offset_reg(EBP, type->offset), x86_reg(EBX))
                ASM(IMUL + 1 + _log2(type->size), x86_reg(EAX), x86_reg(EBX))
                ASM(MOV, x86_reg(EBX), x86_offset_reg(EBP, type->offset))
            }

            if ( !type1->used ) free(type1);
            if ( !type->used ) free(type);
        }
        else {
            expr_prefix_t* prfx = lhs->value;
            // we should sanity check to make sure it's * but w/e
            expr_t* expr = prfx->lhs;
            x86_type_t* type1 = x86_compile_expression(cmp, expr);
            x86_type_t* type2 = x86_compile_expression(cmp, varset->value);

            if ( type1->prim != prim_uint ) {
                free(expr);
                if ( !type2->used ) free(type2);

                x86_error(cmp, "Can only derefence unsigned ints\n");
                goto error;
            }

            ASM(POP, x86_reg(EAX), NULL)
            ASM(POP, x86_reg(EBX), NULL)
            ASM(MOV, x86_reg(EAX), x86_deref_reg(EBX))

            free(type1);
            if ( !type2->used ) free(type2);
        }
    }
    else if ( stmt->type == if_stmt ) {
        if_stmt_t* ifs = stmt->data;
        x86_type_t* type = x86_compile_expression(cmp, ifs->expr);

        if ( type == NULL ) {
            if ( !type->used ) free(type);
            goto error;
        }

        if ( !type->used ) free(type);

        x86_label_t* new_label = (x86_label_t*) malloc(sizeof(x86_label_t));
        new_label->name = (char*) malloc((4 + _log10(cmp->curr_labels) + 1) * sizeof(char));
        sprintf((char*) new_label->name, "_LB%d", cmp->curr_labels++);
        new_label->cmds = INIT_LIST(x86_command_t);
        new_label->cmd_cnt = 0;
        new_label->cmd_alloc = MALLOC_CHUNK;
        new_label->global = false;
        new_label->fn = label->fn;

        ASM(POP, x86_reg(EAX), NULL)
        ASM(CMPL, x86_number(1), x86_reg(EAX))
        ASM(JL,  x86_label(new_label), NULL)

        for ( int i = 0; i < ifs->block->len; i++ ) {
            x86_compile_statement(cmp, ifs->block->stmts[i]);

            if ( cmp->error ) {
                goto error;
            }
        }

        ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})

        cmp->cur_label = new_label;
    }
    else if ( stmt->type == for_stmt ) {
        for_stmt_t* fors = stmt->data;
        x86_compile_statement(cmp, fors->init);

        x86_label_t* body_label = (x86_label_t*) malloc(sizeof(x86_label_t));
        body_label->name = (char*) malloc((4 + _log10(cmp->curr_labels) + 1) * sizeof(char));
        sprintf((char*) body_label->name, "_LB%d", cmp->curr_labels++);
        body_label->cmds = INIT_LIST(x86_command_t);
        body_label->cmd_cnt = 0;
        body_label->cmd_alloc = MALLOC_CHUNK;
        body_label->global = false;
        body_label->fn = label->fn;

        x86_label_t* end_label = (x86_label_t*) malloc(sizeof(x86_label_t));
        end_label->name = (char*) malloc((4 + _log10(cmp->curr_labels) + 1) * sizeof(char));
        sprintf((char*) end_label->name, "_LB%d", cmp->curr_labels++);
        end_label->cmds = INIT_LIST(x86_command_t);
        end_label->cmd_cnt = 0;
        end_label->cmd_alloc = MALLOC_CHUNK;
        end_label->global = false;
        end_label->fn = label->fn;

        ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})
        label = body_label;
        cmp->cur_label = body_label;

        x86_type_t* type = x86_compile_expression(cmp, fors->cond);

        if ( type == NULL ) {
            goto error;
        }

        if ( !type->used ) free(type);

        ASM(POP, x86_reg(EAX), NULL)
        ASM(CMPL, x86_number(1), x86_reg(EAX))
        ASM(JL,  x86_label(end_label), NULL)

        x86_compile_statement(cmp, fors->step);

        for ( int i = 0; i < fors->block->len; i++ ) {
            x86_compile_statement(cmp, fors->block->stmts[i]);

            if ( cmp->error ) {
                goto error;
            }
        }

        ASM(JMP, x86_label(body_label), NULL)

        ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})
        cmp->cur_label = end_label;
    }
    else if ( stmt->type == while_stmt ) {
        while_stmt_t* whs = stmt->data;

        x86_label_t* body_label = (x86_label_t*) malloc(sizeof(x86_label_t));
        body_label->name = (char*) malloc((4 + _log10(cmp->curr_labels) + 1) * sizeof(char));
        sprintf((char*) body_label->name, "_LB%d", cmp->curr_labels++);
        body_label->cmds = INIT_LIST(x86_command_t);
        body_label->cmd_cnt = 0;
        body_label->cmd_alloc = MALLOC_CHUNK;
        body_label->global = false;
        body_label->fn = label->fn;

        x86_label_t* end_label = (x86_label_t*) malloc(sizeof(x86_label_t));
        end_label->name = (char*) malloc((4 + _log10(cmp->curr_labels) + 1) * sizeof(char));
        sprintf((char*) end_label->name, "_LB%d", cmp->curr_labels++);
        end_label->cmds = INIT_LIST(x86_command_t);
        end_label->cmd_cnt = 0;
        end_label->cmd_alloc = MALLOC_CHUNK;
        end_label->global = false;
        end_label->fn = label->fn;

        ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})
        label = body_label;
        cmp->cur_label = body_label;

        x86_type_t* type = x86_compile_expression(cmp, whs->expr);

        if ( type == NULL ) {
            goto error;
        }

        if ( !type->used ) free(type);

        ASM(POP, x86_reg(EAX), NULL)
        ASM(CMPL, x86_number(1), x86_reg(EAX))
        ASM(JL,  x86_label(end_label), NULL)

        for ( int i = 0; i < whs->block->len; i++ ) {
            x86_compile_statement(cmp, whs->block->stmts[i]);

            if ( cmp->error ) {
                goto error;
            }
        }

        ASM(JMP, x86_label(body_label), NULL)

        ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})
        cmp->cur_label = end_label;
    }

    error:
    return;
}

void x86_compile_function(x86_state_t* cmp, function_t* fn) {
    // function_debug(fn);

    int l = x86_find_label(cmp, fn->name);

    if ( l != -1 ) {
        x86_error(cmp, "Unable to compile duplicate function %s\n", fn->name);
        return;
    }

    cmp->cur_label = (x86_label_t*) malloc(sizeof(x86_label_t));
    cmp->cur_label->name = strdup(fn->name);
    cmp->cur_label->cmds = INIT_LIST(x86_command_t);
    cmp->cur_label->cmd_cnt = 0;
    cmp->cur_label->cmd_alloc = MALLOC_CHUNK;
    cmp->cur_label->global = true;
    cmp->cur_label->fn = fn;

    x86_label_t* label = cmp->cur_label;

    ASM(PUSH, x86_reg(EBP), NULL)
    ASM(MOVL, x86_reg(ESP), x86_reg(EBP))
    ASM(SUBL, x86_number(0), x86_reg(ESP))

    int stack_off = 0;

    for ( int i = 0; i < fn->len; i++ ) {
        x86_type_t* type = x86_type(cmp, fn->args[i]->type);

        if ( type == 0 ) {
            x86_error(cmp, "Unknown type in function argument\n");
            goto error;
        }

        cmp->curr_off -= type->size;

        if ( type->is_struct ) {
            cmp->curr_off = -((-cmp->curr_off + 15) & ~15);
        }

        x86_variable_t* var = (x86_variable_t*) malloc(sizeof(x86_variable_t));

        var->name = strdup(fn->args[i]->name);

        if ( var->name == NULL ) {
            x86_error(cmp, "Unable to allocate enough space for a variale name.\n");
            free(var);
            goto error;
        }

        var->offset = cmp->curr_off;
        var->size = type->size;
        var->type = type;

        if ( var->type->prim == prim_struct && var->type->deref_cnt == 0 ) {
            x86_struct_t* struc = cmp->structs[var->type->struct_ind];
            x86_type_t* type = var->type;

            for ( int j = struc->arg_cnt - 1; j >= 0; j-- ) {
                int off = cmp->structs[type->struct_ind]->args[j]->offset + var->offset;
                x86_type_t* ret = cmp->structs[type->struct_ind]->args[j]->type;
                ASM(MOV, x86_offset_reg(EBP, 4 + ((stack_off + 1) *  cmp->structs[type->struct_ind]->args[j]->type->size)), x86_reg(EAX))
                ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, off))
                stack_off += 1;
            }
        }
        else {
            ASM(MOV, x86_offset_reg(EBP, 4 + ((stack_off + 1) * type->size)), x86_reg(EAX))
            ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, cmp->curr_off))

            stack_off += 1;
        }

        ADD_ARG(cmp->variables, cmp->variable_cnt, cmp->variable_alloc, var, {
            free(var);
        })
    }

    for ( int i = 0; i < fn->block->len; i++ ) {
        x86_compile_statement(cmp, fn->block->stmts[i]);

        if ( cmp->error ) {
            goto error;
        }
    }

    if ( cmp->curr_off == 0 ) {
        _x86_label_remove(label, 2);
    }
    else {
        ((x86_number_t*) label->cmds[2]->arg1->data)->number = -cmp->curr_off;
    }

    if ( cmp->cur_label->cmds[cmp->cur_label->cmd_cnt - 1]->inst != RET ) {
        if ( strcmp(fn->ret_type->name, "void") == 0 ) {
            x86_label_t* label = cmp->cur_label;
            ASM(MOV, x86_number(0), x86_reg(EAX))
            ASM(LEAVE, NULL, NULL)
            ASM(RET, NULL, NULL)
        }
        else {
            x86_error(cmp, "No return found for nonvoid function.\n");
            goto error;
        }
    }
    else {
        if ( cmp->cur_label->cmd_cnt == 0 ) {
            if ( strcmp(fn->ret_type->name, "void") == 0 ) {
                x86_label_t* label = cmp->cur_label;
                ASM(MOV, x86_number(0), x86_reg(EAX))
                ASM(LEAVE, NULL, NULL)
                ASM(RET, NULL, NULL)
            }
            else {
                x86_error(cmp, "No return found for nonvoid function.\n");
                goto error;
            }
        }
    }

    ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})

    cmp->cur_label = NULL;

    if ( cmp->variables != NULL ) {
        for ( int i = 0; i < cmp->variable_cnt; i++ ) {
            x86_variable_t* var = cmp->variables[i];

            if ( var == NULL ) continue;

            if ( var->type ) free(var->type);

            free((void*) var->name);
            free(var);
        }

        free(cmp->variables);
    }

    cmp->variables = INIT_LIST(x86_variable_t);
    cmp->variable_cnt = 0;
    cmp->variable_alloc = MALLOC_CHUNK;

    error:
    // idf
    return;
}

void _x86_label_remove(x86_label_t* label, int n) {
    x86_command_t* cmd = label->cmds[n];

    for ( int i = n; i < label->cmd_cnt - 1; i++ ) {
        label->cmds[i] = label->cmds[i+1];
    }

    label->cmds[--label->cmd_cnt] = NULL;

    if ( cmd->arg1 ) {
        free(cmd->arg1->data);
        free(cmd->arg1);
    }
    if ( cmd->arg2 ) {
        free(cmd->arg2->data);
        free(cmd->arg2);
    }
    free(cmd);
}

bool _x86_arg_cmp(x86_arg_t* arg1, x86_arg_t* arg2) {
    if ( arg1->type != arg2->type ) {
        return false;
    }

    if ( arg1->type == arg_reg || arg1->type == arg_deref_reg ) {
        x86_reg_t* d1 = arg1->data;
        x86_reg_t* d2 = arg2->data;

        return d1->reg == d2->reg;
    }
    else if ( arg1->type == arg_number ) {
        x86_number_t* n1 = arg1->data;
        x86_number_t* n2 = arg2->data;

        return n1->number == n2->number;
    }
    else if ( arg1->type == arg_reg_off ) {
        x86_reg_off_t* d1 = (x86_reg_off_t*) arg1->data;
        x86_reg_off_t* d2 = (x86_reg_off_t*) arg2->data;

        return d1->reg == d2->reg && d1->amnt == d2->amnt;
    }

    return false;
}

void x86_label_optimize(x86_label_t* label) {
    if ( label->cmd_cnt == 0 ) {
        return;
    }

    for ( int i = 0; i < label->cmd_cnt - 1; i++ ) {
        x86_command_t* cmd = label->cmds[i];
        x86_command_t* ahead = label->cmds[i + 1];

        if ( (cmd->inst == PUSH || cmd->inst == PUSHL) && (ahead->inst == POP || ahead->inst == POPL) ) {
            if ( _x86_arg_cmp(cmd->arg1, ahead->arg1) ) {
                _x86_label_remove(label, i+1);
                _x86_label_remove(label, i);
            }
            else {
                cmd->inst = (cmd->inst == PUSHL || ahead->inst == POPL) ? MOVL : MOV;
                cmd->arg2 = ahead->arg1;

                ahead->arg1 = NULL;
                _x86_label_remove(label, i + 1);
            }

            i -= 1;
        }
        else if ( (cmd->inst == POP || cmd->inst == POPL) && (ahead->inst == PUSH || ahead->inst == PUSHL) ) {
            if ( _x86_arg_cmp(cmd->arg1, ahead->arg1) ) {
                _x86_label_remove(label, i+1);
                _x86_label_remove(label, i);
            }
            else {
                cmd->inst = (cmd->inst == POPL || ahead->inst == PUSHL) ? MOVL : MOV;
                cmd->arg2 = ahead->arg1;

                ahead->arg1 = NULL;
                _x86_label_remove(label, i + 1);
            }

            i -= 1;
        }
        else if ( cmd->inst == MOV || cmd->inst == MOVL ) {
            if ( ahead->inst == MOV || ahead->inst == MOV ) {
                if ( _x86_arg_cmp(cmd->arg1, ahead->arg2) && _x86_arg_cmp(cmd->arg2, ahead->arg1) ) {
                    _x86_label_remove(label, i + 1);
                    i -= 1;
                }
            }
            else {
                if ( _x86_arg_cmp(cmd->arg1, cmd->arg2) ) {
                    _x86_label_remove(label, i);
                    i -= 1;
                }
            }
        }
    }
}

int x86_resolve_string(x86_state_t* cmp, char* str) {
    for ( int i = 0; i < cmp->string_cnt; i++ ) {
        if ( strcmp(cmp->strings[i], str) == 0 ) {
            return i;
        }
    }

    int index = cmp->string_cnt;

    ADD_ARG(cmp->strings, cmp->string_cnt, cmp->string_alloc, str, {})

    return index;

    error:
    return -1;
}
