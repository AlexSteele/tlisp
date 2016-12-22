#ifndef TLISP_ENV_H_
#define TLISP_ENV_H_

#include "core.h"

typedef struct symtab_entry_t {
    const char *sym;
    struct tlisp_obj_t *obj;
} symtab_entry_t;

typedef struct symtab_t {
    size_t len;
    size_t cap;
    struct symtab_entry_t *entries;
} symtab_t;

typedef struct env_t {
    symtab_t symtab;
    struct env_t *outer;
} env_t;

void env_init(env_t *);
void env_add(env_t *, const char *sym, tlisp_obj_t *);
void env_update(env_t *, const char *sym, tlisp_obj_t *);
tlisp_obj_t *env_find(env_t *, const char *sym);
tlisp_obj_t *env_find_bang(env_t *, const char *sym);

#endif
