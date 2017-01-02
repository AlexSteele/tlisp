#ifndef TLISP_LIST_H_
#define TLISP_LIST_H_

#include "core.h"

typedef tlisp_obj_t list_t;

list_t *list_ins(list_t *, list_t *cons);
list_t *list_ins_at(list_t *, list_t *cons, int);
tlisp_obj_t *list_get(list_t *, int);
list_t *list_rem(list_t *, tlisp_obj_t *);
list_t *list_rem_at(list_t *, int);
int list_len(tlisp_obj_t *);

#endif
