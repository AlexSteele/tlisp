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
    MACRO,
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
    char mark;
} tlisp_obj_t;

void obj_nstr(tlisp_obj_t *, char *out, size_t maxlen);
void print_obj(tlisp_obj_t *);
tlisp_obj_t *new_num(void);
tlisp_obj_t *new_str(void);
tlisp_obj_t *new_sym(void);
tlisp_obj_t *new_cons(void);
tlisp_obj_t *new_lambda(void);
tlisp_obj_t *new_macro(void);

typedef struct line_info_entry_t {
    int line;
    int col;
    tlisp_obj_t *obj;
    struct line_info_entry_t *next;
} line_info_entry_t; 

typedef struct line_info_t {
    char *text;
    line_info_entry_t *entries;
} line_info_t;

void line_info_init(line_info_t *, char *text);
void line_info_add(line_info_t *, tlisp_obj_t *, int line, int col);
void line_info_print(line_info_t *, tlisp_obj_t *);

typedef struct source_t {
    size_t nexpressions;
    size_t cap;
    tlisp_obj_t **expressions;
    line_info_t line_info;
} source_t;

void source_init(source_t *, char *text);
void source_add_expr(source_t *, tlisp_obj_t *expr, int startl, int endl);

#endif
