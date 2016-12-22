#ifndef TLISP_CORE_H_
#define TLISP_CORE_H_

#include <stddef.h>

struct env_t; // Forward declaration.

enum obj_tag_t {
    BOOL,
    NUM,
    STRING,
    SYMBOL,
    CONS,
    NFUNC,
    LAMBDA,
    NIL
};
const char *tag_str(enum obj_tag_t);

typedef struct tlisp_obj_t *(*tlisp_fn)(struct tlisp_obj_t*, struct env_t*);

typedef struct tlisp_obj_t {
    union {
        int num;
        char *str;
        char *sym;
        struct {
            struct tlisp_obj_t *car;
            struct tlisp_obj_t *cdr;
        };
        tlisp_fn fn;
    };
    enum obj_tag_t tag;
} tlisp_obj_t;
void obj_str(tlisp_obj_t *, char *out, size_t maxlen);

tlisp_obj_t *new_num(void);
tlisp_obj_t *new_str(void);
tlisp_obj_t *new_sym(void);
tlisp_obj_t *new_cons(void);

#endif
