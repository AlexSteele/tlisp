
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

enum obj_tag_t {
    NUM,
    STRING,
    SYMBOL,
    FORM,
    CONS,
    FUNC,
    NIL
};

const char *tag_str(enum obj_tag_t t)
{
    switch (t) {
    case NUM: return "NUM";
    case STRING: return "STRING";
    case SYMBOL: return "SYMBOL";
    case FORM: return "FORM";
    case FUNC: return "FUNC";
    case CONS: return "CONS";
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

void env_init(env_t *env)
{
    int i;
    
    env->symtab.len = 0;
    env->symtab.cap = 16;
    env->symtab.entries = malloc(sizeof(symtab_entry_t) * env->symtab.cap);
    for (i = 0; i < env->symtab.cap; i++) {
        env->symtab.entries[i].sym = NULL;
        env->symtab.entries[i].obj = NULL;
    }
    env->outer = NULL;
}

void env_add(env_t *env, const char *sym, struct tlisp_obj_t *obj)
{
    size_t idx = str_hash(sym) % env->symtab.cap;
    symtab_entry_t *entries = env->symtab.entries;

    if (env->symtab.len >= ((env->symtab.cap * 3) / 4)) {
        env->symtab.cap *= 2;
        env->symtab.entries = realloc(env->symtab.entries, env->symtab.cap);
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

typedef struct tlisp_obj_t *(*tlisp_fn)(struct tlisp_obj_t*, env_t*);

typedef struct tlisp_obj_t {
    union {
        float num;
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

tlisp_obj_t *eval(tlisp_obj_t *obj, env_t *env)
{
    switch (obj->tag) {
    case NUM:
        return obj;
    case STRING:
        return obj;
    case SYMBOL:
        return env_find(env, obj->sym);
    case FORM: {
        tlisp_obj_t *fn_obj;

        if (obj->car->tag != SYMBOL) {
            fprintf(stderr, "ERROR: Misplaced symbol. Expecting function.\n");
            exit(1);
        }
        fn_obj = env_find(env, obj->car->sym);
        if (!fn_obj) {
            fprintf(stderr, "ERROR: No function %s\n.", obj->car->sym);
            exit(1);
        }
        return fn_obj->fn(obj->cdr, env);
    }
    }
}

tlisp_obj_t *tlisp_nil;

tlisp_obj_t *tlisp_print(tlisp_obj_t *obj, env_t *env)
{
    switch (obj->tag) {
    case NUM:
        printf("%f\n", obj->num);
        break;
    case STRING:
        printf("%s\n", obj->str);
        break;
    case SYMBOL:
        tlisp_print(env_find(env, obj->sym), env);
        break;
    case FORM:
        tlisp_print(eval(obj, env), env);
        break;
    case CONS: {
        tlisp_print(obj->car, env);
        if (obj->cdr)
            tlisp_print(obj->cdr, env);
        break;
    }
    case FUNC:
        printf("<func>\n");
        break;
    case NIL:
        printf("<nil>\n");
        break;
    }
    return tlisp_nil;
}

void init_genv(env_t *genv)
{
    env_init(genv);
    {
        tlisp_obj_t *pr = malloc(sizeof(tlisp_obj_t));
        pr->fn = tlisp_print;
        pr->tag = FUNC;
        env_add(genv, "print", pr);
    }
    {
        tlisp_nil = malloc(sizeof(tlisp_obj_t));
        tlisp_nil->tag = NIL;
        env_add(genv, "nil", tlisp_nil);
    }
}

tlisp_obj_t *parse_str(char **cursor)
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

tlisp_obj_t *parse_num(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    float f = 0;

    while (*cursor && !whitespace(**cursor)) {
        f *= 10;
        f += **cursor - '0';
        (*cursor)++;
    }
    obj->num = f;
    obj->tag = NUM;
    return obj;
}

tlisp_obj_t *parse_sym(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    char *lead = *cursor;
    size_t len = 0;

    while (*lead && !whitespace(*lead)) {
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

tlisp_obj_t *parse_form(char **cursor)
{
    tlisp_obj_t *head = NULL;
    tlisp_obj_t *curr, *next;
    char c;

    (*cursor)++;
    while ((c = **cursor)) {
        if (whitespace(c))
            continue;
        if (c == ')') 
            break;

        next = malloc(sizeof(tlisp_obj_t));
        next->cdr = NULL;
        next->tag = head ? CONS : FORM;
        if (c == '"') {
            next->car = parse_str(cursor);
        } else if (isdigit(c)) {
            next->car = parse_num(cursor);
        } else if (c == '(') {
            next->car = parse_form(cursor);
        } else {
            next->car = parse_sym(cursor);
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
    return head;
}

tlisp_obj_t **parse(char *raw, size_t *n)
{
    size_t len = 0;
    size_t cap = 128;
    tlisp_obj_t **forms = malloc(sizeof(tlisp_obj_t*) * cap);
    char c;

    while ((c = *raw)) {
        if (whitespace(c)) {
            ;
        } else if (c == '(') {
            tlisp_obj_t *form = parse_form(&raw);
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
            fprintf(stderr, "ERROR: Unexpected symbol %c\n", c);
            exit(1);
        }
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
    tlisp_obj_t **in;
    tlisp_obj_t *res;
    size_t len;

    while (1) {
        printf("tlisp> ");
        fflush(stdout);
        fgets(line, 256, stdin);
        in = parse(line, &len);
        if (len == 0) {
            continue;
        }
        res = eval(in[0], genv);
        tlisp_print(res, genv); 
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
        parse(buff, &len);
    }
    return 0;
}
