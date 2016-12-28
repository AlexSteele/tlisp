
#include "builtins.h"
#include "core.h"
#include "env.h"
#include "process.h"
#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGISTER_NFUNC(sym, func)                      \
    do {                                               \
        tlisp_obj_t *f = malloc(sizeof(tlisp_obj_t));  \
        f->fn = func;                                  \
        f->tag = NFUNC;                                \
        env_add(genv, sym, f);                         \
    } while (0);                                       \

static
void genv_init(env_t *genv, process_t *proc)
{
    env_init(genv, NULL, proc);
    {
        tlisp_nil = malloc(sizeof(tlisp_obj_t));
        tlisp_nil->tag = NIL;
        env_add(genv, "nil", tlisp_nil);
    }
    {
        tlisp_quote = malloc(sizeof(tlisp_obj_t));
        tlisp_quote->tag = SYMBOL;
        tlisp_quote->sym = "'";
    }
    {
        tlisp_backquote = malloc(sizeof(tlisp_obj_t));
        tlisp_backquote->tag = SYMBOL;
        tlisp_backquote->sym = "`";
    }
    {
        tlisp_hashtag = malloc(sizeof(tlisp_obj_t));
        tlisp_hashtag->tag = SYMBOL;
        tlisp_hashtag->sym = "#";
    }
    {
        tlisp_true = malloc(sizeof(tlisp_obj_t));
        tlisp_true->tag = BOOL;
        tlisp_true->num = 1;
        env_add(genv, "true", tlisp_true);
    }
    {
        tlisp_false = malloc(sizeof(tlisp_obj_t));
        tlisp_false->tag = BOOL;
        tlisp_false->num = 0;
        env_add(genv, "false", tlisp_false); 
    }
    REGISTER_NFUNC("eval", tlisp_eval);
    REGISTER_NFUNC("apply", tlisp_apply);
    REGISTER_NFUNC("'", tlisp_quote_fn);
    REGISTER_NFUNC("`", tlisp_backquote_fn);
    REGISTER_NFUNC("type-of", tlisp_type_of);
    REGISTER_NFUNC("let", tlisp_let);
    REGISTER_NFUNC("do", tlisp_do);
    REGISTER_NFUNC("if", tlisp_if);
    REGISTER_NFUNC("while", tlisp_while);
    REGISTER_NFUNC("def", tlisp_def);
    REGISTER_NFUNC("set!", tlisp_set);
    REGISTER_NFUNC("lambda", tlisp_lambda);
    REGISTER_NFUNC("macro", tlisp_macro);
    REGISTER_NFUNC("list", tlisp_list);
    REGISTER_NFUNC("cons", tlisp_cons);
    REGISTER_NFUNC("append", tlisp_append);
    REGISTER_NFUNC("car", tlisp_car);
    REGISTER_NFUNC("cdr", tlisp_cdr);
    REGISTER_NFUNC("len", tlisp_len);
    REGISTER_NFUNC("for-each", tlisp_for_each);
    REGISTER_NFUNC("map", tlisp_map);
    REGISTER_NFUNC("filter", tlisp_filter);
    REGISTER_NFUNC("reduce", tlisp_reduce);
    REGISTER_NFUNC("#", tlisp_dict);
    REGISTER_NFUNC("get", tlisp_get);
    REGISTER_NFUNC("ins", tlisp_ins);
    REGISTER_NFUNC("rem", tlisp_rem);
    REGISTER_NFUNC("+", tlisp_add);
    REGISTER_NFUNC("-", tlisp_sub);
    REGISTER_NFUNC("*", tlisp_mul);
    REGISTER_NFUNC("/", tlisp_div);
    REGISTER_NFUNC("&", tlisp_arith_and);
    REGISTER_NFUNC("|", tlisp_arith_or);
    REGISTER_NFUNC("^", tlisp_xor);
    REGISTER_NFUNC("eq", tlisp_equals);
    REGISTER_NFUNC(">", tlisp_greater_than);
    REGISTER_NFUNC("<", tlisp_less_than);
    REGISTER_NFUNC(">=", tlisp_geq);
    REGISTER_NFUNC("<=", tlisp_leq);
    REGISTER_NFUNC("and", tlisp_and);
    REGISTER_NFUNC("or", tlisp_or);
    REGISTER_NFUNC("not", tlisp_not);
    REGISTER_NFUNC("print", tlisp_print);
    REGISTER_NFUNC("str", tlisp_str);
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
    char line[1024];
    source_t in;
    tlisp_obj_t *res;
    int i;

    while (1) {
        printf("tlisp> ");
        fflush(stdout);
        if (!fgets(line, 1024, stdin)) {
            return 0;
        }
        in = read(line);
        if (in.nexpressions == 0) {
            continue;
        }
        for (i = 0; i < in.nexpressions; i++) {
            res = eval(in.expressions[i], genv);
        }
        print_obj(res);
    }
}

static
int tlisp_file(const char *fname, env_t *genv)
{
    char *buff = read_file(fname);
    source_t source = read(buff);
    size_t i;

    genv->proc->line_info = &source.line_info;
    for (i = 0; i < source.nexpressions; i++) {
        genv->proc->curr_expr = source.expressions[i];
        eval(source.expressions[i], genv);
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
    process_t proc;
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
    proc_init(&proc);
    genv_init(&genv, &proc);
    if (interactive) {
        return tlisp_repl(&genv);
    } 
    return tlisp_file(argv[1], &genv);
}
