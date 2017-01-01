
#include "struct.h"
#include <stdlib.h>
#include <strings.h>

void structdef_destroy(tlisp_structdef_t *sdef)
{
    int i;

    for (i = 0; i < sdef->nfields; i++) {
        free(sdef->field_names[i]);
    }
    free(sdef->field_names);
}

void struct_destroy(tlisp_struct_t *s)
{
    free(s->fields);
}

tlisp_obj_t *struct_get_field(tlisp_struct_t *s, const char *name)
{
    int nfields = s->sdef->nfields;
    char **field_names = s->sdef->field_names;
    int i;

    for (i = 0; i < nfields; i++) {
        if (!strcmp(name, field_names[i])) {
            return s->fields[i];
        }
    }
    return NULL;
}

int struct_setq(tlisp_struct_t *s, const char *name, tlisp_obj_t *to)
{
    tlisp_obj_t *field = struct_get_field(s, name);
    
    if (!field) return 0;

    *(&field) = to;
    return 1;
}
