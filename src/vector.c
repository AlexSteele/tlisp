
#include "core.h"
#include "vector.h"
#include <stdlib.h>

#define MIN_CAP 8

void vec_init(tlisp_vector_t *vec)
{
    vec->len = 0;
    vec->cap = MIN_CAP;
    vec->elems = malloc(sizeof(tlisp_obj_t *) * vec->cap);
}

void vec_destroy(tlisp_vector_t *vec)
{
    free(vec->elems);
}

static
void vec_check_resize(tlisp_vector_t *vec)
{
    if (vec->len == vec->cap) {
        vec->cap *= 2;
        vec->elems = realloc(vec->elems, sizeof(tlisp_obj_t *) * vec->cap);
        return;
    }

    if ((vec->len <= vec->cap / 4) && (vec->cap / 2 >= MIN_CAP)) {
        vec->cap /= 2;
        vec->elems = realloc(vec->elems, sizeof(tlisp_obj_t *) * vec->cap);
        return;
    }
}

void vec_ins(tlisp_vector_t *vec, tlisp_obj_t *obj)
{
    vec_check_resize(vec);
    vec->elems[vec->len] = obj;
    vec->len++;
}

int vec_ins_at(tlisp_vector_t *vec, tlisp_obj_t *obj, int idx)
{
    int i;
    
    if (idx < 0 || idx > vec->len) {
        return 0;
    }
    vec_check_resize(vec);
    for (i = vec->len; i >= idx; i--) {
        vec->elems[i + 1] = vec->elems[i];
    }
    vec->elems[idx] = obj;
    vec->len++;
    return 1;
}

tlisp_obj_t *vec_get(tlisp_vector_t *vec, int idx)
{
    if (idx < 0 || idx >= vec->len) {
        return NULL;
    }
    return vec->elems[idx];
}

int vec_rem(tlisp_vector_t *vec, tlisp_obj_t *obj)
{
    int i;

    for (i = 0; i < vec->len; i++) {
        if (obj_equals(vec->elems[i], obj)) {
            vec_rem_at(vec, i);
            return 1;
        }
    }
    return 0;
}

tlisp_obj_t *vec_rem_at(tlisp_vector_t *vec, int idx)
{
    int i;
    tlisp_obj_t *elem;

    if (idx < 0 || idx >= vec->len) {
        return NULL;
    }
    elem = vec->elems[idx];
    for (i = idx; i < vec->len - 1; i++) {
        vec->elems[i] = vec->elems[i + 1];
    }
    vec->len--;
    vec_check_resize(vec);
    return elem;
}

int vec_len(tlisp_vector_t *vec)
{
    return vec->len;
}

void vec_for_each(tlisp_vector_t *vec, vec_visitor fn, void *state)
{
    int i;

    for (i = 0; i < vec->len; i++) {
        fn(vec->elems[i], state);
    }
}
