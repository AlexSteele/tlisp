
#include "list.h"
#include <assert.h>

list_t *list_ins(list_t *list, list_t *cons)
{
    assert(cons->tag == CONS);
    
    cons->cdr = list;
    return cons;   
}

list_t *list_ins_at(list_t *list, list_t *cons, int idx)
{
    assert(cons->tag == CONS);
    
    if (idx == 0) {
        return list_ins(list, cons);
    } else {
        list_t *curr = list;
        int i = 0;

        while (curr && i < idx - 1) {
            curr = curr->cdr;
            i++;
        }
        if (curr) {
            cons->cdr = curr->cdr;
            curr->cdr = cons;
        }
        return list;
    }    
}

tlisp_obj_t *list_get(list_t *list, int idx)
{
    tlisp_obj_t *curr = list;
    int i = 0;

    while (curr && i < idx) {
        curr = curr->cdr;
        i++;
    }
    return curr ? curr->car : NULL;
}

list_t *list_rem(list_t *list, tlisp_obj_t *obj)
{
    if (obj_equals(list->car, obj)) {
        return list->cdr;
    } else {
        list_t *curr = list;

        while (curr->cdr) {
            if (obj_equals(curr->cdr->car, obj)) {
                curr->cdr = curr->cdr->cdr;
            }
            curr = curr->cdr;
        }
        return list;
    }
}

list_t *list_rem_at(list_t *list, int idx)
{
    if (idx == 0) {
        return list->cdr;
    } else {
        list_t *curr = list;
        int i = 0;

        while (curr && i < idx - 1) {
            curr = curr->cdr;
            i++;
        }
        if (curr && curr->cdr) {
            curr->cdr = curr->cdr->cdr;
        }
        return list;
    }
    return 0;
}

int list_len(list_t *list)
{
    int len = 0;

    while (list) {
        list = list->cdr;
        len++;
    }
    return len;
}
