
#include "process.h"
#include "dict.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
tlisp_obj_t *new_obj(process_t *proc)
{
    tlisp_obj_t *obj;
    
    if (proc->heap_len == proc->heap_cap) {
        tlisp_obj_t *heap;
        
        proc->heap_cap *= 2;
        heap = malloc(sizeof(tlisp_obj_t) * proc->heap_cap);
        memcpy(heap, proc->heap, sizeof(tlisp_obj_t) * proc->heap_len);
        free(proc->heap);
        proc->heap = heap;
    }
    obj = proc->heap + proc->heap_len;
    obj->mark = 0;
    proc->heap_len++;
    return obj;
}

void proc_init(process_t *proc)
{
    proc->nalive = 0;
    proc->heap_len = 0;
    proc->heap_cap = (MIN_HEAP_SIZE + sizeof(tlisp_obj_t) - 1) / sizeof(tlisp_obj_t);
    proc->heap = malloc(sizeof(tlisp_obj_t) * proc->heap_cap);
    proc->curr_expr = NULL;
}

void proc_fatal(process_t *proc, const char *msg)
{
    fprintf(stderr, "%s", msg);
    fflush(stderr);
    if (proc->curr_expr) {
        line_info_print(proc->line_info, proc->curr_expr);
    }
    exit(1);
}

#define DEF_CONSTRUCTOR(abbrev, tag_)                   \
    tlisp_obj_t *proc_new_##abbrev(process_t *proc)     \
    {                                                   \
        tlisp_obj_t *obj = new_obj(proc);               \
        obj->tag = tag_;                                \
        return obj;                                     \
    }                                                   \
    
DEF_CONSTRUCTOR(str, STRING)
DEF_CONSTRUCTOR(sym, SYMBOL)
DEF_CONSTRUCTOR(num, NUM)
DEF_CONSTRUCTOR(lambda, LAMBDA)
DEF_CONSTRUCTOR(macro, MACRO)
DEF_CONSTRUCTOR(structdef, STRUCTDEF)
DEF_CONSTRUCTOR(struct, STRUCT)

tlisp_obj_t *proc_new_cons(process_t *proc)
{
    tlisp_obj_t *obj = new_obj(proc);
    obj->tag = CONS;
    obj->cdr = NULL; 
    return obj;
}

tlisp_obj_t *proc_new_dict(process_t *proc)
{
    tlisp_obj_t *obj = new_obj(proc);
    obj->tag = DICT;
    dict_init(&obj->dict);
    return obj;
}

tlisp_obj_t *proc_new_vec(process_t *proc)
{
    tlisp_obj_t *obj = new_obj(proc);
    obj->tag = VEC;
    vec_init(&obj->vec);
    return obj;
}

static
int proc_fcheck(process_t *proc, tlisp_obj_t *fobj)
{
    return fobj->tag == NUM &&
        fobj->num >= 0 &&
        fobj->num < proc->nfiles;
}

tlisp_obj_t *proc_open(process_t *proc, const char *fname, const char *mode)
{
    FILE *f;
    tlisp_obj_t *fobj;

    if (proc->nfiles == MAX_FILES) {
        return NULL;
    }
    f = fopen(fname, mode);
    if (!f) {
        return NULL;
    }
    fobj = proc_new_num(proc);
    fobj->num = proc->nfiles;
    proc->ftable[proc->nfiles] = f;
    proc->nfiles++;
    return fobj;
}

FILE *proc_getf(process_t *proc, tlisp_obj_t *fobj)
{
    if (!proc_fcheck(proc, fobj)) {
        return NULL;
    }
    return proc->ftable[fobj->num];
}

int proc_close(process_t *proc, tlisp_obj_t *fobj)
{
    FILE *f;
    int res;
    int i;

    if (!proc_fcheck(proc, fobj)) {
        return 0;
    }
    f = proc->ftable[fobj->num];
    res = fclose(f) != EOF;
    for (i = fobj->num + 1; i < proc->nfiles; i++) {
        proc->ftable[i - 1] = proc->ftable[i];
    }
    proc->nfiles--;
    return res;
}
