
#include "list.h"

tlisp_obj_t *list_get(tlisp_obj_t *list, int idx)
{
    tlisp_obj_t *curr = list;

    while (curr && (curr - list) < idx) {
        curr = curr->cdr;
    }
    return curr ? curr->car : NULL;
}

int list_len(tlisp_obj_t *list)
{
    int len = 0;

    while (list) {
        len++;
        list = list->cdr;
    }
    return len;
}
