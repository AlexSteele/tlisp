
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

enum obj_tag_t {
    NUM,
    STRING,
    SYMBOL,
    LIST
};

typedef struct tlisp_obj_t {
    union {
        float num;
        char *str;
        char *sym;
        struct {
            struct tlisp_obj_t *car;
            struct tlisp_obj_t *cdr;
        };
    };
    enum obj_tag_t tag;
} tlisp_obj_t;

void eval(tlisp_obj_t *obj)
{
    switch (obj->tag) {
    case NUM:

        break;
    case STRING:

        break;
    case SYMBOL:

        break;
    case LIST:

        break;
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
    return 0;
}

tlisp_obj_t *parse_sym(char **cursor)
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    char *lead = *cursor;
    size_t len = 0;

    while (*lead && *lead != ' ' && *lead != '\t' && *lead != '\n') {
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
        if (c == ' ' || c == '\t' || c == '\n') 
            continue;
        if (c == ')') 
            break;
        
        next = malloc(sizeof(tlisp_obj_t));
        next->cdr = NULL;
        next->tag = LIST;
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
            next->cdr = curr;
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
        if (c == ' ' || c == '\t' || c == '\n') {
            continue;
        } else if (c == '(') {
            tlisp_obj_t *form = parse_form(&raw);
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
    return buff;
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
    if (interactive) {
        print_usage(argv[0]);
        return 0;
    }

    char *buff = read_file(argv[1]);
    size_t len;
    parse(buff, &len);
    return 0;
}
