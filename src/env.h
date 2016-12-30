#ifndef TLISP_ENV_H_
#define TLISP_ENV_H_

#include "core.h"
#include "process.h"

typedef struct symtab_entry_t {
    const char *sym;
    tlisp_obj_t *obj;
} symtab_entry_t;

typedef struct symtab_t {
    size_t len;
    size_t cap;
    symtab_entry_t *entries;
} symtab_t;

typedef struct env_t {
    symtab_t symtab;
    process_t *proc;
    struct env_t *outer;
} env_t;

typedef void (*env_visitor)(tlisp_obj_t *, void *);

void env_init(env_t *, env_t *outer, process_t *);
void env_destroy(env_t *); 
void env_add(env_t *, const char *sym, tlisp_obj_t *);
tlisp_obj_t *env_find(env_t *, const char *sym);
int env_update(env_t *, const char *sym, tlisp_obj_t *);
void env_for_each(env_t *, env_visitor, void *);

#endif
