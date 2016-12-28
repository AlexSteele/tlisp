
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *tag_str(enum obj_tag_t t)
{
    switch (t) {
    case BOOL: return "bool";
    case NUM: return "num";
    case STRING: return "string";
    case SYMBOL: return "symbol";
    case NFUNC: return "nfunc";
    case CONS: return "cons";
    case LAMBDA: return "lambda";
    case MACRO: return "macro";
    case NIL: return "nil";
    }
}

void obj_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
{
    switch (obj->tag) {
    case BOOL:
        snprintf(str, maxlen, "%s", obj->num ? "true" : "false");
        break;
    case NUM:
        snprintf(str, maxlen, "%d", obj->num);
        break;
    case STRING:
        snprintf(str, maxlen, "%s", obj->str);
        break;
    case SYMBOL:
        snprintf(str, maxlen, "%s", obj->sym);
        break;
    case CONS: {
#define REMAINING (maxlen - (tail - str))
        char *tail = str;

        if (maxlen < 3)
            return;
        tail[0] = '(';
        tail++;
        while (obj && REMAINING > 2) {
            obj_nstr(obj->car, tail, REMAINING - 2);
            while (*tail && REMAINING > 2) {
                tail++;
            }
            obj = obj->cdr;
            if (obj && REMAINING > 2) {
                tail[0] = ' ';
                tail++;
            }
        }
        tail[0] = ')';
        tail[1] = 0;
        break;
#undef REMAINING
    }
    case NFUNC:
        strncpy(str, "<native func>", maxlen);
        break;
    case LAMBDA:
        strncpy(str, "<lambda>", maxlen);
        break;
    case MACRO:
        strncpy(str, "<macro>", maxlen);
        break;
    case NIL:
        strncpy(str, "nil", maxlen);
        break;
    }
}

void print_obj(tlisp_obj_t *obj)
{
    char str[1024];
    obj_nstr(obj, str, 1024);
    printf("%s\n", str);
}

#define DEF_CONSTRUCTOR(abbrev, tag_)                   \
    tlisp_obj_t *new_##abbrev()                         \
    {                                                   \
        tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t)); \
        obj->tag = tag_;                                \
        obj->mark = 0;                                  \
        return obj;                                     \
    }                                                   \
    
DEF_CONSTRUCTOR(str, STRING)
DEF_CONSTRUCTOR(sym, SYMBOL)
DEF_CONSTRUCTOR(num, NUM)
DEF_CONSTRUCTOR(lambda, LAMBDA)
DEF_CONSTRUCTOR(macro, MACRO)

tlisp_obj_t *new_cons()
{
    tlisp_obj_t *obj = malloc(sizeof(tlisp_obj_t));
    obj->tag = CONS;
    obj->mark = 0;
    obj->cdr = NULL; 
    return obj;
}

void line_info_init(line_info_t *info, char *text)
{
    info->text =text;
    info->entries = NULL;
}

void line_info_add(line_info_t *info, tlisp_obj_t *obj, int line, int col)
{
    line_info_entry_t *entry = malloc(sizeof(line_info_entry_t));

    entry->obj = obj;
    entry->line = line;
    entry->col = col;
    entry->next = info->entries;
    info->entries = entry;
}

static
line_info_entry_t *find_entry(line_info_t *info, tlisp_obj_t *obj)
{
    line_info_entry_t *entry = info->entries;

    while (entry) {
        if (entry->obj == obj) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static
void print_line(line_info_t *info, int num)
{
    char *cursor = info->text;
    int curr = 1;

    while (curr < num) {
        cursor = strchr(cursor, '\n');
        cursor++;
        curr++;
    }
    while (*cursor && *cursor != '\n') {
        fprintf(stderr, "%c", *cursor);
        cursor++;
    }
    fprintf(stderr, "\n");
}

void line_info_print(line_info_t *info, tlisp_obj_t *obj)
{
    line_info_entry_t *entry = find_entry(info, obj);

    if (!entry) return;
    fprintf(stderr, "Line %d\n", entry->line);
    print_line(info, entry->line);
}

void source_init(source_t *source, char *text)
{
    line_info_init(&source->line_info, text);
    source->cap = 16;
    source->expressions = malloc(sizeof(tlisp_obj_t*) * source->cap);
    source->nexpressions = 0;
}

void source_add_expr(source_t *source, tlisp_obj_t *expr,
                     int start_line, int end_line)
{
    if (source->nexpressions == source->cap) {
        source->cap *= 2;
        source->expressions = realloc(source->expressions, source->cap);
    }
    line_info_add(&source->line_info, expr, start_line, 1);
    source->expressions[source->nexpressions] = expr;
    source->nexpressions++;
}
