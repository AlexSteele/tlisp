
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>

static
int is_true(tlisp_obj_t *obj)
{
    return obj == tlisp_true;
}

static
int c_bool(tlisp_obj_t *obj)
{
    return is_true(obj) ? 1 : 0;
}

static
int nargs(tlisp_obj_t *args)
{
    int a = 0;
    while (args) {
        a++;
        args = args->cdr;
    }
    return a;
}

static
void assert_type(tlisp_obj_t *obj, enum obj_tag_t expected)
{
    if (obj->tag != expected) {
        char objstr[128];
        obj_str(obj, objstr, 128);
        fprintf(stderr, "ERROR: Bad type for object %s. Expected %s.\n",
                objstr, tag_str(expected));
        exit(1);
    }
}

static
void assert_nargs(int n, tlisp_obj_t *args)
{
    int a = nargs(args);
    if (a != n) {
        fprintf(stderr,
                "ERROR: Wrong number of arguments. Got %d. Expected %d",
                a, n);
        exit(1);
    }
}

static
tlisp_obj_t *arg_at(int idx, tlisp_obj_t *args)
{
    int i = 0;
    while (i < idx) {
        args = args->cdr;
        i++;
    }
    return args->car;
}

static
void eval_args(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        args->car = tlisp_eval(args->car, env);
        args = args->cdr;
    }
}

tlisp_obj_t *tlisp_eval(tlisp_obj_t *args, env_t *env)
{
    // TODO: FINISH IMPL
    switch (args->tag) {
    case BOOL:
        return args;
    case NUM:
        return args;
    case STRING:
        return args;
    case SYMBOL:
        return env_find_bang(env, args->sym);
    case CONS: {
        tlisp_obj_t *fn = env_find_bang(env, args->car->sym);
        tlisp_obj_t *fn_args = args->cdr;
        return fn->fn(fn_args, env);
    }
    case NFUNC:
        return 0;
    case LAMBDA:
        return 0;
    case NIL:
        return args;
    }
}

tlisp_obj_t *tlisp_do(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        tlisp_eval(args->car, env);
        args = args->cdr;
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_if(tlisp_obj_t *args, env_t *env)
{
    int len = nargs(args);

    if (len != 2 && len != 3) {
        fprintf(stderr, "ERROR: Bad if. Wrong number of args (%d).\n", len);
        exit(1);
    }
    if (is_true(tlisp_eval(arg_at(0, args), env))) {
        return tlisp_eval(arg_at(1, args), env);
    }
    if (len == 3) {
        return tlisp_eval(arg_at(2, args), env);
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_while(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *cond;
    tlisp_obj_t *body;

    assert_nargs(2, args);
    cond = arg_at(0, args);
    body = arg_at(1, args);

    while (is_true(tlisp_eval(cond, env))) {
        tlisp_eval(body, env);
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_def(tlisp_obj_t *args, env_t *env)
{
    return 0;
}

tlisp_obj_t *tlisp_set(tlisp_obj_t *args, env_t *env)
{
    return 0;
}

tlisp_obj_t *tlisp_cons(tlisp_obj_t *args, env_t *env)
{
    return 0;
}

tlisp_obj_t *tlisp_car(tlisp_obj_t *args, env_t *env)
{
    return 0;
}

tlisp_obj_t *tlisp_cdr(tlisp_obj_t *args, env_t *env)
{
    return 0;
}

/* TODO: fix */
#define DEF_ARITH_OP(name, op)                                 \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *args, env_t *env)   \
    {                                                          \
        tlisp_obj_t *res = new_num();                          \
                                                               \
        res->num = 0;                                          \
        eval_args(args, env);                                  \
        while (args) {                                         \
            assert_type(args->car, NUM);                       \
            res->num op##= args->car->num;                     \
            args = args->cdr;                                  \
        }                                                      \
        return res;                                            \
    }                                                          \

DEF_ARITH_OP(add, +)
DEF_ARITH_OP(sub, -)
DEF_ARITH_OP(mul, *)
DEF_ARITH_OP(div, /)
DEF_ARITH_OP(arith_and, &)
DEF_ARITH_OP(arith_or, |)
DEF_ARITH_OP(xor, ^)

#define DEF_CMP_OP(name, op)                                    \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *args, env_t *env)    \
    {                                                           \
        tlisp_obj_t *arg_a, *arg_b;                             \
        int a, b;                                               \
                                                                \
        assert_nargs(2, args);                                  \
        eval_args(args, env);                                   \
        arg_a = arg_at(0, args);                                \
        arg_b = arg_at(1, args);                                \
        assert_type(arg_a, NUM);                                \
        assert_type(arg_b, NUM);                                \
                                                                \
        a = arg_at(0, args)->num;                               \
        b = arg_at(1, args)->num;                               \
        return (a op b) ? tlisp_true : tlisp_false;             \
    }                                                           \

DEF_CMP_OP(equals, ==)
DEF_CMP_OP(greater_than, >)
DEF_CMP_OP(less_than, <)
DEF_CMP_OP(geq, >=)
DEF_CMP_OP(leq, <=)

/* TODO: Be more flexible in arg types and number. */
#define DEF_BOOL_OP(name, op)                                           \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *args, env_t *env)            \
    {                                                                   \
        tlisp_obj_t *arg_a, *arg_b;                                     \
        int a, b;                                                       \
                                                                        \
        assert_nargs(2, args);                                          \
        eval_args(args, env);                                           \
        arg_a = arg_at(0, args);                                        \
        arg_b = arg_at(1, args);                                        \
        assert_type(arg_a, BOOL);                                       \
        assert_type(arg_b, BOOL);                                       \
                                                                        \
        a = c_bool(arg_a);                                              \
        b = c_bool(arg_b);                                              \
        return (a op b) ? tlisp_true : tlisp_false;                     \
    }                                                                   \
    
DEF_BOOL_OP(and, &&)
DEF_BOOL_OP(or, ||)

tlisp_obj_t *tlisp_not(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *arg;
    
    assert_nargs(1, args);
    arg = arg_at(0, args);
    assert_type(arg, BOOL);
    return c_bool(arg) ? tlisp_false : tlisp_true; 
}

tlisp_obj_t *tlisp_print(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        char str[256];
        tlisp_obj_t *curr;

        curr = tlisp_eval(args->car, env);
        obj_str(curr, str, 256);
        printf("%s\n", str);
        args = args->cdr;
    }
    return tlisp_nil;
}
