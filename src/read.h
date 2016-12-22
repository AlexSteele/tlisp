#ifndef TLISP_READ_H_
#define TLISP_READ_H_

#include "builtins.h"
#include "core.h"

tlisp_obj_t **read(char *source, size_t *out_len);

#endif
