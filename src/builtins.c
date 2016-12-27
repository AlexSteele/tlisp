
#include "builtins.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
tlisp_obj_t *tlisp_bool(int c_bool)
{
    return c_bool ? tlisp_true : tlisp_false;
}

static
tlisp_obj_t *num_cpy(tlisp_obj_t *obj)
{
    tlisp_obj_t *res = new_num();
    res->num = obj->num;
    return res;
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
void assert_fn(tlisp_obj_t *obj, process_t *proc)
{
    if (obj->tag != NFUNC && obj->tag != LAMBDA) {
        char errstr[256];
        char objstr[128];
        obj_nstr(obj, objstr, 128);
        snprintf(errstr, 256,
                 "ERROR: Wrong type for %s. Expected function.\n", objstr);
        proc_fatal(proc, errstr);
    }
}

static
void assert_type(tlisp_obj_t *obj, enum obj_tag_t expected, process_t *proc)
{
    if (obj->tag != expected) {
        char errstr[256];
        char objstr[128];
        obj_nstr(obj, objstr, 128);
        snprintf(errstr, 256, "ERROR: Wrong type for %s (%s). Expected %s.\n",
                 objstr, tag_str(obj->tag), tag_str(expected));
        proc_fatal(proc, errstr);
    }
}

static
void assert_nargs(int n, tlisp_obj_t *args, process_t *proc)
{
    int a = nargs(args);
    if (a != n) {
        char errstr[256];
        snprintf(errstr, 256,
                 "ERROR: Wrong number of arguments. Got %d. Expected %d.\n", a, n);
        proc_fatal(proc, errstr);
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
tlisp_obj_t *call_fn(tlisp_obj_t *fn, tlisp_obj_t *args, env_t *env)
{
    if (fn->tag == NFUNC) {
        return fn->fn(args, env);
    } else {
        env_t inner_env;
        tlisp_obj_t *res;
        env_init(&inner_env, env, env->proc);
        res = apply(fn, args, &inner_env);
        env_destroy(&inner_env);
        return res;
    }
}

static
tlisp_obj_t *call_1arity_fn(tlisp_obj_t *fn, tlisp_obj_t *arg, env_t *env)
{
    tlisp_obj_t arglist;
    arglist.tag = CONS;
    arglist.car = arg;
    arglist.cdr = NULL;
    return call_fn(fn, &arglist, env);
}

static
tlisp_obj_t *call_2arity_fn(tlisp_obj_t *fn,
                            tlisp_obj_t *arg1, tlisp_obj_t *arg2,
                            env_t *env)
{
    tlisp_obj_t cons1;
    tlisp_obj_t cons2;
    cons1.tag = CONS;
    cons1.car = arg1;
    cons1.cdr = &cons2;
    cons2.tag = CONS;
    cons2.car = arg2;
    cons2.cdr = NULL;
    return call_fn(fn, &cons1, env);
}

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env)
{
    switch (obj->tag) {
    case BOOL:
    case NUM:
    case STRING:
    case NIL:
        return obj;
    case SYMBOL: {
        tlisp_obj_t *o = env_find(env, obj->sym);
        if (!o) {
            char errstr[256];
            snprintf(errstr, 256, "ERROR: Undefined symbol '%s'.\n", obj->sym);
            proc_fatal(env->proc, errstr);
        }
        return o;
    }
    case CONS:
        return tlisp_apply(obj, env);
    case NFUNC:
    case LAMBDA:
        fprintf(stderr, "ERROR: eval called on function.\n");
        exit(1);
    }
}

tlisp_obj_t *apply(tlisp_obj_t *fn, tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *arg_list = fn->car;
    tlisp_obj_t *body = fn->cdr;
    tlisp_obj_t *res;

    while (arg_list || args) {
        if (!arg_list) {
            proc_fatal(env->proc, "ERROR: Too many arguments.\n");
        }
        if (!args) {
            proc_fatal(env->proc, "ERROR: Too few arguments.\n");
        }
        env_add(env, arg_list->car->sym, eval(args->car, env));
        arg_list = arg_list->cdr;
        args = args->cdr;
    }
    while (body) {
        res = eval(body->car, env);
        body = body->cdr;
    }
    return res;
}

tlisp_obj_t *tlisp_eval(tlisp_obj_t *args, env_t *env)
{
    assert_nargs(1, args, env->proc);
    if (args->car->tag == CONS &&
        args->car->car == tlisp_quote) {
        return eval(args->car->cdr, env);
    }
    return eval(args->car, env);
}

tlisp_obj_t *tlisp_apply(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *fn;
    tlisp_obj_t *fn_args;

    if (!args) {
        proc_fatal(env->proc, "ERROR: apply requires at least one argument.\n");
    }
    fn = eval(args->car, env);
    fn_args = args->cdr;
    assert_fn(fn, env->proc);
    return call_fn(fn, fn_args, env);
}

tlisp_obj_t *tlisp_quote_fn(tlisp_obj_t *args, env_t *env)
{
    if (!args) {
        proc_fatal(env->proc, "ERROR: quote requires at least one argument.\n");
    }
    return args;
}

tlisp_obj_t *tlisp_type_of(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *res;
    tlisp_obj_t *arg;
    
    assert_nargs(1, args, env->proc);
    arg = eval(args->car, env);
    res = proc_new_str(env->proc);
    res->str = strdup(tag_str(arg->tag));
    return res;
}

tlisp_obj_t *tlisp_do(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *res = NULL;
    
    while (args) {
        res = eval(args->car, env);
        args = args->cdr;
    }
    return res ? res : tlisp_nil;
}

tlisp_obj_t *tlisp_if(tlisp_obj_t *args, env_t *env)
{
    int len = nargs(args);

    if (len != 2 && len != 3) {
        proc_fatal(env->proc, "ERROR: Invalid if expression.\n");
    }
    if (is_true(eval(arg_at(0, args), env))) {
        return eval(arg_at(1, args), env);
    }
    if (len == 3) {
        return eval(arg_at(2, args), env);
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_while(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *cond;
    tlisp_obj_t *body;

    assert_nargs(2, args, env->proc);
    cond = arg_at(0, args);
    body = arg_at(1, args);

    while (is_true(eval(cond, env))) {
        eval(body, env);
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_def(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *sym;
    tlisp_obj_t *val;
    
    assert_nargs(2, args, env->proc);
    sym = arg_at(0, args);
    val = arg_at(1, args);
    assert_type(sym, SYMBOL, env->proc);

    val = eval(val, env);
    env_add(env, sym->sym, val);
    return val;
}

tlisp_obj_t *tlisp_set(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *sym;
    tlisp_obj_t *val;

    assert_nargs(2, args, env->proc);
    sym = arg_at(0, args);
    val = arg_at(1, args);
    assert_type(sym, SYMBOL, env->proc);

    val = eval(val, env);
    env_update(env, sym->sym, val);
    return val;
}

tlisp_obj_t *tlisp_lambda(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *res = proc_new_lambda(env->proc);
    
    if (!args || !args->cdr) {
        proc_fatal(env->proc, "ERROR: lambda requires at least two arguments.\n");
    }
    res->car = args->car;
    res->cdr = args->cdr;
    assert_type(args->car, CONS, env->proc);
    args = args->car;
    while (args) {
        assert_type(args->car, SYMBOL, env->proc);
        args = args->cdr;
    }
    return res;
}

tlisp_obj_t *tlisp_list(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *head;
    tlisp_obj_t *curr;

    if (!args) {
        return tlisp_nil;
    }
    head = proc_new_cons(env->proc);
    head->car = eval(args->car, env);
    curr = head;
    while ((args = args->cdr)) {
        curr->cdr = proc_new_cons(env->proc);
        curr = curr->cdr;
        curr->car = eval(args->car, env);
    }
    return head;
}

tlisp_obj_t *tlisp_cons(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *res;

    assert_nargs(2, args, env->proc);
    res = proc_new_cons(env->proc);
    res->car = eval(args->car, env);
    res->cdr = eval(args->cdr->car, env);
    if (res->cdr == tlisp_nil) {
        res->cdr = NULL;
        return res;
    }
    assert_type(res->cdr, CONS, env->proc);
    return res;
}

tlisp_obj_t *tlisp_append(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *res;
    tlisp_obj_t *head;

    assert_nargs(2, args, env->proc);
    res = proc_new_cons(env->proc);
    res->car = eval(args->car, env);
    head = eval(args->cdr->car, env);
    if (head == tlisp_nil) {
        return res;
    }
    args = head;
    while (args->cdr) {
        args = args->cdr;
    }
    args->cdr = res;
    return head;
}

tlisp_obj_t *tlisp_car(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;

    assert_nargs(1, args, env->proc);
    list = eval(args->car, env);
    if (list == tlisp_nil)  {
        return tlisp_nil;
    }
    assert_type(list, CONS, env->proc);
    return list->car;
}

tlisp_obj_t *tlisp_cdr(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;

    assert_nargs(1, args, env->proc);
    list = eval(args->car, env);
    assert_type(list, CONS, env->proc);
    return list->cdr ? list->cdr : tlisp_nil;
}

tlisp_obj_t *tlisp_len(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;
    tlisp_obj_t *res;
    int len = 0;

    assert_nargs(1, args, env->proc);
    list = eval(args->car, env);
    res = proc_new_num(env->proc);
    if (list == tlisp_nil) {
        res->num = 0;
        return res;
    }
    assert_type(list, CONS, env->proc);
    while (list) {
        len += 1;
        list = list->cdr;
    }
    res->num = len;
    return res;
}

tlisp_obj_t *tlisp_for_each(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;
    tlisp_obj_t *fn;

    assert_nargs(2, args, env->proc);
    list = eval(args->car, env);
    if (list == tlisp_nil) {
        return 0;
    }
    assert_type(list, CONS, env->proc);
    fn = eval(args->cdr->car, env);
    assert_fn(fn, env->proc);
    while (list) {
        call_1arity_fn(fn, list->car, env);
        list = list->cdr;
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_map(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;
    tlisp_obj_t *fn;
    tlisp_obj_t *res;
    tlisp_obj_t *curr;

    assert_nargs(2, args, env->proc);
    list = eval(args->car, env);
    if (list == tlisp_nil) {
        return tlisp_nil;
    }
    assert_type(list, CONS, env->proc);
    fn = eval(args->cdr->car, env);
    assert_fn(fn, env->proc);
    res = proc_new_cons(env->proc);
    res->car = call_1arity_fn(fn, list->car, env);
    curr = res;
    while ((list = list->cdr)) {
        curr->cdr = proc_new_cons(env->proc);
        curr = curr->cdr;
        curr->car = call_1arity_fn(fn, list->car, env);
    }
    return res;
}

tlisp_obj_t *tlisp_filter(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;
    tlisp_obj_t *fn;
    tlisp_obj_t *res = NULL;
    tlisp_obj_t *curr;

    assert_nargs(2, args, env->proc);
    list = eval(args->car, env);
    if (list == tlisp_nil) {
        return tlisp_nil;
    }
    assert_type(list, CONS, env->proc);
    fn = eval(args->cdr->car, env);
    assert_fn(fn, env->proc);
    while (list) {
        tlisp_obj_t *keep = call_1arity_fn(fn, list->car, env);
        if (is_true(keep)) {
            if (res == NULL) {
                res = proc_new_cons(env->proc);
                res->car = list->car;
                curr = res;
            } else {
                curr->cdr = proc_new_cons(env->proc);
                curr = curr->cdr;
                curr->car = list->car;
            }
        }
        list = list->cdr;
    }
    return res ? res : tlisp_nil;
}

tlisp_obj_t *tlisp_reduce(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *list;
    tlisp_obj_t *fn;
    tlisp_obj_t *res = NULL;

    assert_nargs(2, args, env->proc);
    list = eval(args->car, env);
    if (list == tlisp_nil) {
        return tlisp_nil;
    }
    assert_type(list, CONS, env->proc);
    fn = eval(args->cdr->car, env);
    assert_fn(fn, env->proc);
    if (!list->cdr) {
        return list->car;
    }
    res = call_2arity_fn(fn, list->car, list->cdr->car, env);
    list = list->cdr->cdr;
    while (list) {
        res = call_2arity_fn(fn, res, list->car, env);
        list = list->cdr;
    }
    return res;
}

#define DEF_ARITH_OP(name, op)                                 \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *args, env_t *env)   \
    {                                                          \
        tlisp_obj_t *res;                                      \
                                                               \
        if (!args) {                                           \
            char errstr[256];                                  \
            snprintf(errstr, 256,                              \
                     "ERROR: Too few args to %s.\n",           \
                     #op);                                     \
            proc_fatal(env->proc, errstr);                     \
        }                                                      \
        res = args->car->tag == NUM ?                          \
            num_cpy(args->car) :                               \
            eval(args->car, env);                              \
        assert_type(res, NUM, env->proc);                      \
        while ((args = args->cdr)) {                           \
            tlisp_obj_t *curr = eval(args->car, env);          \
            assert_type(curr, NUM, env->proc);                 \
            res->num op##= curr->num;                          \
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
        assert_nargs(2, args, env->proc);                       \
        arg_a = eval(arg_at(0, args), env);                     \
        arg_b = eval(arg_at(1, args), env);                     \
        assert_type(arg_a, NUM, env->proc);                     \
        assert_type(arg_b, NUM, env->proc);                     \
                                                                \
        a = arg_a->num;                                         \
        b = arg_b->num;                                         \
        return (a op b) ? tlisp_true : tlisp_false;             \
    }                                                           \

DEF_CMP_OP(greater_than, >)
DEF_CMP_OP(less_than, <)
DEF_CMP_OP(geq, >=)
DEF_CMP_OP(leq, <=)

tlisp_obj_t *tlisp_equals(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *arg_a, *arg_b;

    assert_nargs(2, args, env->proc);
    arg_a = eval(arg_at(0, args), env);
    arg_b = eval(arg_at(1, args), env);
    if (arg_a->tag != arg_b->tag) {
        return tlisp_false;
    }
    switch (arg_a->tag) {
    case BOOL:
        return tlisp_bool(arg_a == arg_b);
    case NUM:
        return tlisp_bool(arg_a->num == arg_b->num);
    case STRING:
        return tlisp_bool(!strcmp(arg_a->str, arg_b->str));
    case SYMBOL:
        return tlisp_bool(!strcmp(arg_a->sym, arg_b->sym));
    case CONS:
        return tlisp_bool(arg_a == arg_b);
    case NFUNC:
        return tlisp_bool(arg_a->fn == arg_b->fn);
    case LAMBDA:
        return tlisp_bool(arg_a == arg_b);
    case NIL:
        return tlisp_bool(arg_a == arg_b);
    }
}

/* TODO: Be more flexible in arg types and number. */
#define DEF_BOOL_OP(name, op)                                     \
    tlisp_obj_t *tlisp_##name(tlisp_obj_t *args, env_t *env)      \
    {                                                             \
        tlisp_obj_t *arg_a, *arg_b;                               \
        int a, b;                                                 \
                                                                  \
        assert_nargs(2, args, env->proc);                         \
        arg_a = eval(arg_at(0, args), env);                       \
        arg_b = eval(arg_at(1, args), env);                       \
        assert_type(arg_a, BOOL, env->proc);                      \
        assert_type(arg_b, BOOL, env->proc);                      \
                                                                  \
        a = c_bool(arg_a);                                        \
        b = c_bool(arg_b);                                        \
        return (a op b) ? tlisp_true : tlisp_false;               \
    }                                                             \
    
DEF_BOOL_OP(and, &&)
DEF_BOOL_OP(or, ||)

tlisp_obj_t *tlisp_not(tlisp_obj_t *args, env_t *env)
{
    tlisp_obj_t *arg;
    
    assert_nargs(1, args, env->proc);
    arg = eval(args->car, env);
    assert_type(arg, BOOL, env->proc);
    return c_bool(arg) ? tlisp_false : tlisp_true; 
}

tlisp_obj_t *tlisp_print(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        char str[1024];
        tlisp_obj_t *curr;

        curr = eval(args->car, env);
        obj_nstr(curr, str, 1024);
        printf("%s\n", str);
        args = args->cdr;
    }
    return tlisp_nil;
}

tlisp_obj_t *tlisp_str(tlisp_obj_t *args, env_t *env)
{
    char arg_strs[1024];
    tlisp_obj_t *res = proc_new_str(env->proc);
    int res_len = 0;

    if (!args) {
        res->str = strdup("");
        return res;
    }
    while (args) {
        tlisp_obj_t *curr = eval(args->car, env);
        obj_nstr(curr, arg_strs + res_len, 1024 - res_len - 1);
        res_len += strlen(arg_strs + res_len);
        args = args->cdr;
    }
    arg_strs[res_len++] = 0;
    res->str = malloc(sizeof(char) * res_len);
    strcpy(res->str, arg_strs);
    return res;
}
