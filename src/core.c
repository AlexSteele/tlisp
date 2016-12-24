
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *tag_str(enum obj_tag_t t)
{
    switch (t) {
    case BOOL: return "bool";
    case NUM: return "num";
    case STRING: return "string";
    case SYMBOL: return "symbol";
    case NFUNC: return "nfunc";
    case CONS: return "cons";
    case LAMBDA: return "lambda";
    case NIL: return "nil";
    }
}

void obj_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
{
    switch (obj->tag) {
    case BOOL:
        snprintf(str, maxlen, "%s", obj->num ? "true" : "false");
        break;
    case NUM:
        snprintf(str, maxlen, "%d", obj->num);
        break;
    case STRING:
        snprintf(str, maxlen, "%s", obj->str);
        break;
    case SYMBOL:
        snprintf(str, maxlen, "<symbol %s>", obj->sym);
        break;
    case CONS:
        strncpy(str, "<cons>", maxlen);
        break;
    case NFUNC:
        strncpy(str, "<native func>", maxlen);
        break;
    case LAMBDA:
        strncpy(str, "<lambda>", maxlen);
        break;
    case NIL:
        strncpy(str, "nil", maxlen);
        break;
    }
}

#define DEF_CONSTRUCTOR(abbrev, tag_)                   \
    tlisp_obj_t *new_##abbrev()                         \
    {                                                   \
        tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t)); \
        obj->tag = tag_;                                \
        return obj;                                     \
    }                                                   \
    
DEF_CONSTRUCTOR(str, STRING)
DEF_CONSTRUCTOR(sym, SYMBOL)
DEF_CONSTRUCTOR(num, NUM)
DEF_CONSTRUCTOR(lambda, LAMBDA)

tlisp_obj_t *new_cons()
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    obj->tag = CONS;
    obj->cdr = NULL; 
    return obj;
}
