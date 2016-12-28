#ifndef TLISP_DICT_H_
#define TLISP_DICT_H_

#include "core.h"

typedef struct tlisp_dict_entry_t {
    tlisp_obj_t *key;
    tlisp_obj_t *val;
    int valid;
} tlisp_dict_entry_t;

typedef struct tlisp_dict_t {
    int len;
    int cap;
    tlisp_dict_entry_t *entries;
} tlisp_dict_t;

typedef void (*dict_visitor)(tlisp_obj_t *key, tlisp_obj_t *val, void *);

tlisp_dict_t *dict_new(void);
void dict_free(tlisp_dict_t *);
tlisp_obj_t *dict_ins(tlisp_dict_t *, tlisp_obj_t *, tlisp_obj_t *);
tlisp_obj_t *dict_get(tlisp_dict_t *, tlisp_obj_t *);
tlisp_obj_t *dict_rem(tlisp_dict_t *, tlisp_obj_t *);
int dict_len(tlisp_dict_t *);
void dict_for_each(tlisp_dict_t *, dict_visitor, void *);

#endif
