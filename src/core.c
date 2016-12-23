
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *tag_str(enum obj_tag_t t)
{
    switch (t) {
    case BOOL: return "BOOL";
    case NUM: return "NUM";
    case STRING: return "STRING";
    case SYMBOL: return "SYMBOL";
    case NFUNC: return "NFUNC";
    case CONS: return "CONS";
    case LAMBDA: return "LAMBDA";
    case NIL: return "NIL";
    }
}

void obj_str(tlisp_obj_t *obj, char *str, size_t maxlen)
{
    switch (obj->tag) {
    case BOOL:
        snprintf(str, maxlen, "<bool %s>", obj->num ? "true" : "false");
        break;
    case NUM:
        snprintf(str, maxlen, "<num %d>", obj->num);
        break;
    case STRING:
        snprintf(str, maxlen, "<string \"%s\">", obj->str);
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
    case NIL:
        strncpy(str, "<nil>", maxlen);
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
