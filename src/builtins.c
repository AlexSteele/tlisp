
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>

static
int true_bool(tlisp_obj_t *obj)
{
    return obj == tlisp_true ? 1 : 0;
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
        fprintf(stderr, "ERROR: Wrong number of arguments. Got %d. Expected %d",
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
    }
    return args->car;
}

#define DEF_ARITH_OP(name, op)                                 \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *arg, env_t *env)    \
    {                                                          \
        if (arg->tag == NUM) {                                 \
            return arg;                                        \
        } else {                                               \
            tlisp_obj_t *res;                                  \
            assert_type(arg->car, NUM);                        \
            res = malloc(sizeof(tlisp_obj_t));                 \
            res->tag = NUM;                                    \
            res->num = arg->car->num;                          \
            while ((arg = arg->cdr)) {                         \
                assert_type(arg->car, NUM);                    \
                res->num op##= arg->car->num;                  \
            }                                                  \
            return res;                                        \
        }                                                      \
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
        int a, b;                                               \
        assert_nargs(2, args);                                  \
        assert_type(arg_at(0, args), NUM);                      \
        assert_type(arg_at(1, args), NUM);                      \
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
        int p, q;                                                       \
        assert_nargs(2, args);                                          \
        assert_type(arg_at(0, args), BOOL);                             \
        assert_type(arg_at(1, args), BOOL);                             \
        p = true_bool(arg_at(0, args));                                 \
        q = true_bool(arg_at(1, args));                                 \
        return (p op q) ? tlisp_true : tlisp_false;                     \
    }                                                                   \
    
DEF_BOOL_OP(and, &&)
DEF_BOOL_OP(or, ||)

tlisp_obj_t *tlisp_not(tlisp_obj_t *args, env_t *env)
{
    assert_nargs(1, args);
    assert_type(arg_at(0, args), BOOL);
    return arg_at(0, args) == tlisp_true ?
        tlisp_false : tlisp_true; 
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

tlisp_obj_t *tlisp_print(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        char str[256];
        obj_str(args->car, str, 256);
        printf("%s\n", str);
        args = args->cdr;
    }
    return tlisp_nil;
}
