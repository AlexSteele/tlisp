#ifndef TLISP_BUILTINS_H_
#define TLISP_BUILTINS_H_

#include "core.h"
#include "env.h"

tlisp_obj_t *tlisp_nil;
tlisp_obj_t *tlisp_quote;
tlisp_obj_t *tlisp_true;
tlisp_obj_t *tlisp_false;

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *);
tlisp_obj_t *apply(tlisp_obj_t *fn, tlisp_obj_t *args, env_t *);

tlisp_obj_t *tlisp_eval(tlisp_obj_t *args, env_t *);
tlisp_obj_t *tlisp_apply(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_quote_fn(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_type_of(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_let(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_do(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_if(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_while(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_def(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_set(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_lambda(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_list(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_cons(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_append(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_car(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_cdr(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_len(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_for_each(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_map(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_filter(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_reduce(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_add(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_sub(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_mul(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_div(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_arith_and(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_arith_or(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_xor(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_equals(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_greater_than(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_less_than(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_geq(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_leq(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_and(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_or(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_not(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_print(tlisp_obj_t *, env_t *);
tlisp_obj_t *tlisp_str(tlisp_obj_t *, env_t *);

#endif
