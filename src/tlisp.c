
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum obj_tag_t {
    BOOL,
    NUM,
    STRING,
    SYMBOL,
    FORM,
    CONS,
    NFUNC,
    LAMBDA,
    NIL
};

const char *tag_str(enum obj_tag_t t)
{
    switch (t) {
    case BOOL: return "BOOL";
    case NUM: return "NUM";
    case STRING: return "STRING";
    case SYMBOL: return "SYMBOL";
    case FORM: return "FORM";
    case NFUNC: return "NFUNC";
    case CONS: return "CONS";
    case LAMBDA: return "LAMBDA";
    case NIL: return "NIL";
    }
}

struct tlisp_obj_t;

typedef struct symtab_entry_t {
    const char *sym;
    struct tlisp_obj_t *obj;
} symtab_entry_t;

typedef struct symtab_t {
    size_t len;
    size_t cap;
    struct symtab_entry_t *entries;
} symtab_t;

typedef struct env_t {
    symtab_t symtab;
    struct env_t *outer;
} env_t;

size_t str_hash(const char *s)
{
    size_t hash = 5381;
    while (*s) {
        hash = (hash << 5) + hash + *s;
        s++;
    }
    return hash;
}

void env_zero(env_t *env, int low, int high)
{
    int i;
    for (i = low; i < high; i++) {
        env->symtab.entries[i].sym = NULL;
        env->symtab.entries[i].obj = NULL;
    }
}

void env_add(env_t *env, const char *sym, struct tlisp_obj_t *obj);
void env_grow(env_t *env)
{
    int i;
    symtab_entry_t *old = env->symtab.entries;
    
    env->symtab.cap *= 2;
    env->symtab.entries = malloc(sizeof(symtab_entry_t) * env->symtab.cap);
    env_zero(env, 0, env->symtab.cap);
    for (i = 0; i < env->symtab.cap / 2; i++) {
        if (old[i].sym) {
            env_add(env, old[i].sym, old[i].obj);
        }
    }
    free(old);
}

void env_init(env_t *env)
{
    env->symtab.len = 0;
    env->symtab.cap = 16;
    env->symtab.entries = malloc(sizeof(symtab_entry_t) * env->symtab.cap);
    env_zero(env, 0, env->symtab.cap);
    env->outer = NULL;
}

void env_add(env_t *env, const char *sym, struct tlisp_obj_t *obj)
{
    size_t idx = str_hash(sym) % env->symtab.cap;
    symtab_entry_t *entries = env->symtab.entries;

    if (env->symtab.len >= ((env->symtab.cap * 3) / 4)) {
        env_grow(env);
        entries = env->symtab.entries;
    }
    while (entries[idx].sym) {
        if (!strcmp(sym, entries[idx].sym)) {
            fprintf(stderr, "ERROR: Duplicate symbol definition %s.\n", sym);
            exit(1);
        }
        idx = (idx + 1) % env->symtab.cap;
    }
    entries[idx].sym = sym;
    entries[idx].obj = obj;
    env->symtab.len++;
}

struct tlisp_obj_t *env_find(env_t *env, const char *sym)
{
    size_t hash = str_hash(sym);
    
    while (env) {
        symtab_entry_t *entries = env->symtab.entries;
        size_t idx = hash % env->symtab.cap;
        while (entries[idx].sym) {
            if (!strcmp(entries[idx].sym, sym)) {
                return entries[idx].obj;
            }
            idx = (idx + 1) % env->symtab.cap;
        }
        env = env->outer;
    }
    return NULL;
}

struct tlisp_obj_t *env_find_bang(env_t *env, const char *sym)
{
    struct tlisp_obj_t *obj = env_find(env, sym);
    if (!obj) {
        fprintf(stderr, "ERROR: Undefined symbol '%s'.\n", sym);
        exit(1);
    }
    return obj;
}

typedef struct tlisp_obj_t *(*tlisp_fn)(struct tlisp_obj_t*, env_t*);

typedef struct tlisp_obj_t {
    union {
        int num;
        char *str;
        char *sym;
        struct {
            struct tlisp_obj_t *car;
            struct tlisp_obj_t *cdr;
        };
        tlisp_fn fn;
    };
    enum obj_tag_t tag;
} tlisp_obj_t;

/* TODO: REMOVE EXTRA FORWARD DECL. */
tlisp_obj_t *tlisp_true; 
void obj_str(tlisp_obj_t *obj, char *str, size_t maxlen)
{
    switch (obj->tag) {
    case BOOL:
        snprintf(str, maxlen, "<bool %s>", obj == tlisp_true ? "true" : "false");
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
    case FORM:
        strncpy(str, "<form>", maxlen);
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

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env);
void eval_args(tlisp_obj_t *args, env_t *env)
{
    while (args) {
        args->car = eval(args->car, env);
        args = args->cdr;
    }
}

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env)
{
    switch (obj->tag) {
    case BOOL:
        return obj;
    case NUM:
        return obj;
    case STRING:
        return obj;
    case SYMBOL: {
        return env_find_bang(env, obj->sym);
    }
    case FORM: {
        tlisp_obj_t *fn_obj = env_find_bang(env, obj->car->sym);
        eval_args(obj->cdr, env);
        return fn_obj->fn(obj->cdr, env);
    }
    case CONS:
        return 0;
    case NFUNC:
        return 0;
    case LAMBDA:
        return 0;
    case NIL:
        return obj;
    }
}

int nargs(tlisp_obj_t *args)
{
    int a = 0;
    while (args) {
        a++;
        args = args->cdr;
    }
    return a;
}

void assert_nargs(int n, tlisp_obj_t *args)
{
    int a = nargs(args);
    if (a != n) {
        fprintf(stderr, "ERROR: Wrong number of arguments. Got %d. Expected %d",
                a, n);
        exit(1);
    }
}

tlisp_obj_t *arg_at(int idx, tlisp_obj_t *args)
{
    int i = 0;
    while (i < idx) {
        args = args->cdr;
    }
    return args->car;
}

/* BUILTINS */

tlisp_obj_t *tlisp_nil;
tlisp_obj_t *tlisp_true;
tlisp_obj_t *tlisp_false;

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
        return (a op b) ? tlisp_true : tlisp_false;            \
    }                                                           \

DEF_CMP_OP(equals, ==)
DEF_CMP_OP(greater_than, >)
DEF_CMP_OP(less_than, <)
DEF_CMP_OP(geq, >=)
DEF_CMP_OP(leq, <=)

int true_bool(tlisp_obj_t *obj)
{
    return obj == tlisp_true ? 1 : 0;
}

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

#define REGISTER_NFUNC(sym, func)                      \
    do {                                               \
        tlisp_obj_t *f = malloc(sizeof(tlisp_obj_t));  \
        f->fn = func;                                  \
        f->tag = NFUNC;                                \
        env_add(genv, sym, f);                         \
    } while (0);                                       \

void init_genv(env_t *genv)
{
    env_init(genv);
    {
        tlisp_nil = malloc(sizeof(tlisp_obj_t));
        tlisp_nil->tag = NIL;
        env_add(genv, "nil", tlisp_nil);
    }
    {
        tlisp_true = malloc(sizeof(tlisp_obj_t));
        tlisp_true->tag = BOOL;
        env_add(genv, "true", tlisp_true);
    }
    {
        tlisp_false = malloc(sizeof(tlisp_obj_t));
        tlisp_false->tag = BOOL;
        env_add(genv, "false", tlisp_false); 
    }
    REGISTER_NFUNC("+", tlisp_add);
    REGISTER_NFUNC("-", tlisp_sub);
    REGISTER_NFUNC("*", tlisp_mul);
    REGISTER_NFUNC("/", tlisp_div);
    REGISTER_NFUNC("&", tlisp_arith_and);
    REGISTER_NFUNC("|", tlisp_arith_or);
    REGISTER_NFUNC("^", tlisp_xor);
    REGISTER_NFUNC("eq", tlisp_equals);
    REGISTER_NFUNC(">", tlisp_greater_than);
    REGISTER_NFUNC("<", tlisp_less_than);
    REGISTER_NFUNC(">=", tlisp_geq);
    REGISTER_NFUNC("<=", tlisp_leq);
    REGISTER_NFUNC("and", tlisp_and);
    REGISTER_NFUNC("or", tlisp_or);
    REGISTER_NFUNC("def", tlisp_def);
    REGISTER_NFUNC("set!", tlisp_set);
    REGISTER_NFUNC("cons", tlisp_cons);
    REGISTER_NFUNC("car", tlisp_car);
    REGISTER_NFUNC("cdr", tlisp_cdr);
    REGISTER_NFUNC("print", tlisp_print);
}

/* READER */

int whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

tlisp_obj_t *read_num(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    int f = 0;

    while (*cursor && !whitespace(**cursor) && **cursor != ')') {
        f *= 10;
        f += **cursor - '0';
        (*cursor)++;
    }
    obj->num = f;
    obj->tag = NUM;
    return obj;
}

tlisp_obj_t *read_str(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    char *lead;
    size_t len = 0;

    (*cursor)++;
    lead = *cursor;
    while (*lead && *lead != '"') {
        len++;
        lead++;
    }
    obj->str = malloc(sizeof(char) * (len + 1));
    memcpy(obj->str, *cursor, len);
    obj->sym[len] = 0;
    obj->tag = STRING;
    return obj;
}

tlisp_obj_t *read_sym(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    char *lead = *cursor;
    size_t len = 0;

    while (*lead && !whitespace(*lead) && *lead != ')') {
        len++;
        lead++;
    }
    obj->sym = malloc(sizeof(char) * (len + 1));
    memcpy(obj->sym, *cursor, len);
    obj->sym[len] = 0;
    obj->tag = SYMBOL;
    (*cursor) += len;
    return obj;
}

tlisp_obj_t *read_form(char **cursor)
{
    tlisp_obj_t *head = NULL;
    tlisp_obj_t *curr, *next;
    char c;

    (*cursor)++;
    while ((c = **cursor)) {
        if (whitespace(c)) {
            (*cursor)++;
            continue;
        }
        if (c == ')') {
            break;
        }

        next = malloc(sizeof(tlisp_obj_t));
        next->cdr = NULL;
        next->tag = head ? CONS : FORM;
        if (c == '"') {
            next->car = read_str(cursor);
        } else if (isdigit(c)) {
            next->car = read_num(cursor);
       } else if (c == '(') {
            next->car = read_form(cursor);
        } else {
            next->car = read_sym(cursor);
        }
        if (head) {
            curr->cdr = next;
        } else {
            head = next;
        }
        curr = next;
        (*cursor)++;
    }
    (*cursor)++;
    assert_type(head->car, SYMBOL);
    return head;
}

tlisp_obj_t **read(char *raw, size_t *n)
{
    size_t len = 0;
    size_t cap = 128;
    tlisp_obj_t **forms = malloc(sizeof(tlisp_obj_t*) * cap);
    char c;

    while ((c = *raw)) {
        if (whitespace(c)) {
            ;
        } else if (c == '(') {
            tlisp_obj_t *form = read_form(&raw);
            if (!form) {
                fprintf(stderr, "ERROR: Empty form.\n");
                exit(1);
            }
            if (len == cap) {
                cap *= 2;
                forms = realloc(forms, sizeof(tlisp_obj_t*) * cap);
            }
            forms[len] = form;
            len++;
        } else {
            fprintf(stderr, "ERROR: Unexpected symbol '%c'\n", c);
            exit(1);
        }
        printf("%c\n", c);
        raw++;
    }
    *n = len;
    return forms;
}

char *read_file(const char *fname)
{
    size_t cap = 8192;
    size_t len = 0;
    char *buff = malloc(sizeof(char) * cap);
    FILE *fin = fopen(fname, "r");
    
    if (!fin) {
        fprintf(stderr, "ERROR: Unable to open %s.\n", fname);
        exit(1);
    }
    while (!feof(fin)) {
        char c = fgetc(fin);
        if (len == cap) {
            cap *= 2;
            buff = realloc(buff, sizeof(char) * cap); 
        }
        buff[len] = c;
        len++;
    }
    fclose(fin);
    return buff;
}

int tlisp_repl(env_t *genv)
{
    char line[256];
    char res_str[256];
    tlisp_obj_t **in;
    tlisp_obj_t *res;
    size_t len;

    while (1) {
        printf("tlisp> ");
        fflush(stdout);
        fgets(line, 256, stdin);
        in = read(line, &len);
        if (len == 0) {
            continue;
        }
        res = eval(in[0], genv);
        obj_str(res, res_str, 256);
        printf("%s\n", res_str); 
    }
    return 0;
}

void print_usage(const char *progname)
{
    printf("USAGE: %s [options] [file]\n", progname);
    printf("\t-h Print this help message\n");
    printf("\t-i Run interactive REPL\n");
}

int main(int argc, char **argv)
{
    int i;
    int help = 0;
    int interactive = 0;
    env_t genv;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h"))
            help = 1;
        if (!strcmp(argv[i], "-i"))
            interactive = 1;
    }
    if (help) {
        print_usage(argv[0]);
        return 0;
    }
    init_genv(&genv);
    if (interactive) {
        return tlisp_repl(&genv);
    } else {
        char *buff = read_file(argv[1]);
        size_t len;
        read(buff, &len);
    }
    return 0;
}
