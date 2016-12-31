#ifndef TLISP_VECTOR_H_
#define TLISP_VECTOR_H_

typedef struct tlisp_obj_t tlisp_obj_t;

typedef struct tlisp_vector_t {
    int len;
    int cap;
    tlisp_obj_t **elems;
} tlisp_vector_t;

typedef void (*vec_visitor)(tlisp_obj_t *, void *);

void vec_init(tlisp_vector_t *);
void vec_destroy(tlisp_vector_t *);
void vec_ins(tlisp_vector_t *, tlisp_obj_t *);
int vec_ins_at(tlisp_vector_t *, tlisp_obj_t *, int);
tlisp_obj_t *vec_get(tlisp_vector_t *, int);
int vec_rem(tlisp_vector_t *, tlisp_obj_t *);
tlisp_obj_t *vec_rem_at(tlisp_vector_t *, int);
int vec_len(tlisp_vector_t *);
void vec_for_each(tlisp_vector_t *, vec_visitor, void *);

#endif
