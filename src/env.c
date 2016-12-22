
#include "env.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
size_t str_hash(const char *s)
{
    size_t hash = 5381;
    while (*s) {
        hash = (hash << 5) + hash + *s;
        s++;
    }
    return hash;
}

static
void env_grow(env_t *env)
{
    int i;
    size_t old_cap = env->symtab.cap;
    symtab_entry_t *old = env->symtab.entries;
    
    env->symtab.cap *= 2;
    env->symtab.entries = calloc(env->symtab.cap, sizeof(symtab_entry_t));
    for (i = 0; i < old_cap; i++) {
        if (old[i].sym) {
            env_add(env, old[i].sym, old[i].obj);
        }
    }
    free(old);
}

void env_init(env_t *env)
{
    env->symtab.len = 0;
    env->symtab.cap = 16;
    env->symtab.entries = calloc(env->symtab.cap, sizeof(symtab_entry_t));
    env->outer = NULL;
}

void env_add(env_t *env, const char *sym, struct tlisp_obj_t *obj)
{
    size_t idx = str_hash(sym) % env->symtab.cap;
    symtab_entry_t *entries = env->symtab.entries;

    if (env->symtab.len >= ((env->symtab.cap * 3) / 4)) {
        env_grow(env);
        entries = env->symtab.entries;
    }
    while (entries[idx].sym) {
        if (!strcmp(sym, entries[idx].sym)) {
            fprintf(stderr, "ERROR: Duplicate symbol definition %s.\n", sym);
            exit(1);
        }
        idx = (idx + 1) % env->symtab.cap;
    }
    entries[idx].sym = sym;
    entries[idx].obj = obj;
    env->symtab.len++;
}

tlisp_obj_t *env_find(env_t *env, const char *sym)
{
    size_t hash = str_hash(sym);
    
    while (env) {
        symtab_entry_t *entries = env->symtab.entries;
        size_t idx = hash % env->symtab.cap;
        while (entries[idx].sym) {
            if (!strcmp(entries[idx].sym, sym)) {
                return entries[idx].obj;
            }
            idx = (idx + 1) % env->symtab.cap;
        }
        env = env->outer;
    }
    return NULL;
}

tlisp_obj_t *env_find_bang(env_t *env, const char *sym)
{
    struct tlisp_obj_t *obj = env_find(env, sym);
    if (!obj) {
        fprintf(stderr, "ERROR: Undefined symbol '%s'.\n", sym);
        exit(1);
    }
    return obj;
}

#define REGISTER_NFUNC(sym, func)                      \
    do {                                               \
        tlisp_obj_t *f = malloc(sizeof(tlisp_obj_t));  \
        f->fn = func;                                  \
        f->tag = NFUNC;                                \
        env_add(genv, sym, f);                         \
    } while (0);                                       \

void init_genv(env_t *genv)
{
    env_init(genv);
    {
        tlisp_nil = malloc(sizeof(tlisp_obj_t));
        tlisp_nil->tag = NIL;
        env_add(genv, "nil", tlisp_nil);
    }
    {
        tlisp_true = malloc(sizeof(tlisp_obj_t));
        tlisp_true->tag = BOOL;
        env_add(genv, "true", tlisp_true);
    }
    {
        tlisp_false = malloc(sizeof(tlisp_obj_t));
        tlisp_false->tag = BOOL;
        env_add(genv, "false", tlisp_false); 
    }
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
    REGISTER_NFUNC("def", tlisp_def);
    REGISTER_NFUNC("set!", tlisp_set);
    REGISTER_NFUNC("cons", tlisp_cons);
    REGISTER_NFUNC("car", tlisp_car);
    REGISTER_NFUNC("cdr", tlisp_cdr);
    REGISTER_NFUNC("print", tlisp_print);
}
