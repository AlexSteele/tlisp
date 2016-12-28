
#include "env.h"
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

void env_init(env_t *env, env_t *outer, process_t *proc)
{
    env->symtab.len = 0;
    env->symtab.cap = 16;
    env->symtab.entries = calloc(env->symtab.cap, sizeof(symtab_entry_t));
    env->proc = proc;
    env->outer = outer;
}

void env_destroy(env_t *env)
{
    free(env->symtab.entries);
}

void env_add(env_t *env, const char *sym, tlisp_obj_t *obj)
{
    size_t hash = str_hash(sym);
    size_t idx = hash % env->symtab.cap;
    symtab_entry_t *entries = env->symtab.entries;

    if (env->symtab.len >= ((env->symtab.cap * 3) / 4)) {
        env_grow(env);
        idx = hash % env->symtab.cap;
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

symtab_entry_t *env_find_internal(env_t *env, const char *sym)
{
    size_t hash = str_hash(sym);
    
    while (env) {
        symtab_entry_t *entries = env->symtab.entries;
        size_t idx = hash % env->symtab.cap;
        
        while (entries[idx].sym) {
            if (!strcmp(entries[idx].sym, sym)) {
                return &entries[idx];
            }
            idx = (idx + 1) % env->symtab.cap;
        }
        env = env->outer;
    }
    return NULL;
}

tlisp_obj_t *env_find(env_t *env, const char *sym)
{
    symtab_entry_t *entry = env_find_internal(env, sym);
    return entry ? entry->obj : NULL;
}

void env_update(env_t *env, const char *sym, tlisp_obj_t *obj)
{
    symtab_entry_t *entry = env_find_internal(env, sym);

    if (!entry) {
        fprintf(stderr,
                "ERROR: Cannot update %s. No previous definition.\n",
                sym);
        exit(1);
    }
    entry->obj = obj;
}

void env_for_each(env_t *env, env_visitor fn, void *state)
{
    while (env) {
        symtab_entry_t *entries = env->symtab.entries;
        int cap = env->symtab.cap;
        int i;

        for (i = 0; i < cap; i++) {
            if (entries[i].sym) {
                fn(entries[i].obj, state);
            }
        }
        env = env->outer;
    }
}
