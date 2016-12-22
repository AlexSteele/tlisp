
#include "core.h"
#include "env.h"
#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env);
void eval_args(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        args->car = eval(args->car, env);
        args = args->cdr;
    }
}

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env)
{
    // TODO: FINISH IMPL
    switch (obj->tag) {
    case BOOL:
        return obj;
    case NUM:
        return obj;
    case STRING:
        return obj;
    case SYMBOL: 
        return env_find_bang(env, obj->sym);
    case CONS: {
        tlisp_obj_t *fn_obj = env_find_bang(env, obj->car->sym);
        eval_args(obj->cdr, env);
        return fn_obj->fn(obj->cdr, env);
    }
    case NFUNC:
        return 0;
    case LAMBDA:
        return 0;
    case NIL:
        return obj;
    }
}

static
char *read_file(const char *fname)
{
    size_t cap = 8192;
    size_t len = 0;
    char *buff = malloc(sizeof(char) * cap);
    FILE *fin = fopen(fname, "r");
    char c;
    
    if (!fin) {
        fprintf(stderr, "ERROR: Unable to open %s.\n", fname);
        exit(1);
    }
    while ((c = fgetc(fin)) != EOF) {
        if (len == cap) {
            cap *= 2;
            buff = realloc(buff, sizeof(char) * cap); 
        }
        buff[len] = c;
        len++;
    }
    fclose(fin);
    return buff;
}

static
int tlisp_repl(env_t *genv)
{
    char line[256];
    char res_str[256];
    tlisp_obj_t **in;
    tlisp_obj_t *res;
    size_t len;

    while (1) {
        printf("tlisp> ");
        fflush(stdout);
        fgets(line, 256, stdin);
        in = read(line, &len);
        if (len == 0) {
            continue;
        }
        res = eval(in[0], genv);
        obj_str(res, res_str, 256);
        printf("%s\n", res_str); 
    }
    return 0;
}

static
void print_usage(const char *progname)
{
    printf("USAGE: %s [options] [file]\n", progname);
    printf("\t-h Print this help message\n");
    printf("\t-i Run interactive REPL\n");
}

int main(int argc, char **argv)
{
    int i;
    int help = 0;
    int interactive = 0;
    env_t genv;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h"))
            help = 1;
        if (!strcmp(argv[i], "-i"))
            interactive = 1;
    }
    if (help) {
        print_usage(argv[0]);
        return 0;
    }
    init_genv(&genv);
    if (interactive) {
        return tlisp_repl(&genv);
    } else {
        char *buff = read_file(argv[1]);
        size_t len;
        read(buff, &len);
    }
    return 0;
}
