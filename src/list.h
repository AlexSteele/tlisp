#ifndef TLISP_LIST_H_
#define TLISP_LIST_H_

// List Utilities - 
//     These are approached slightly differently than the other
//     collections, since the cons cell is embedded within tlisp_obj_t.

#include "core.h"

tlisp_obj_t *list_get(tlisp_obj_t *, int);
int list_len(tlisp_obj_t *);

#endif
