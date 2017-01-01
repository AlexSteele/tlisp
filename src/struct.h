#ifndef TLISP_STRUCT_H_
#define TLISP_STRUCT_H_

typedef struct tlisp_obj_t tlisp_obj_t;

typedef struct tlisp_structdef_t {
    char *name;
    int nfields;
    char **field_names;
} tlisp_structdef_t;

void structdef_destroy(tlisp_structdef_t *);

typedef struct tlisp_struct_t {
    tlisp_structdef_t *sdef;
    tlisp_obj_t **fields;
} tlisp_struct_t;

void struct_destroy(tlisp_struct_t *);
tlisp_obj_t *struct_get_field(tlisp_struct_t *, const char *);
int struct_setq(tlisp_struct_t *, const char *, tlisp_obj_t *);

#endif
