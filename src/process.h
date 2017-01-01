#ifndef TLISP_PROCESS_H_
#define TLISP_PROCESS_H_

#include "core.h"
#include <stdio.h>

#define MIN_HEAP_SIZE 256000000 /* 256 MB */
#define MAX_FILES 128

typedef struct process_t {
    size_t nalive;
    size_t heap_len;
    size_t heap_cap;
    tlisp_obj_t *heap;
    line_info_t *line_info;
    tlisp_obj_t *curr_expr;
    int nfiles;
    FILE *ftable[MAX_FILES];
} process_t;

void proc_init(process_t *);
void proc_fatal(process_t *, const char *);
tlisp_obj_t *proc_new_num(process_t *);
tlisp_obj_t *proc_new_str(process_t *);
tlisp_obj_t *proc_new_sym(process_t *);
tlisp_obj_t *proc_new_structdef(process_t *);
tlisp_obj_t *proc_new_struct(process_t *);
tlisp_obj_t *proc_new_cons(process_t *);
tlisp_obj_t *proc_new_lambda(process_t *);
tlisp_obj_t *proc_new_macro(process_t *);
tlisp_obj_t *proc_new_dict(process_t *);
tlisp_obj_t *proc_new_vec(process_t *);
tlisp_obj_t *proc_open(process_t *, const char *, const char *);
FILE *proc_getf(process_t *, tlisp_obj_t *);
int proc_close(process_t *, tlisp_obj_t *);

#endif
