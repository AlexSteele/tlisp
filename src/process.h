#ifndef TLISP_PROCESS_H_
#define TLISP_PROCESS_H_

#include "core.h"

#define MIN_HEAP_SIZE 256000000 /* 256 MB */

typedef struct process_t {
    size_t nalive;
    size_t heap_len;
    size_t heap_cap;
    tlisp_obj_t *heap;
    line_info_t *line_info;
    tlisp_obj_t *curr_expr;
} process_t;

void proc_init(process_t *);
void proc_fatal(process_t *, const char *);
tlisp_obj_t *proc_new_num(process_t *);
tlisp_obj_t *proc_new_str(process_t *);
tlisp_obj_t *proc_new_sym(process_t *);
tlisp_obj_t *proc_new_cons(process_t *);
tlisp_obj_t *proc_new_lambda(process_t *);
tlisp_obj_t *proc_new_macro(process_t *);
tlisp_obj_t *proc_new_dict(process_t *);

#endif
