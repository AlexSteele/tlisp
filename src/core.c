
#include "core.h"
#include "dict.h"
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
    case DICT: return "dict";
    case VEC: return "vector";
    case LAMBDA: return "lambda";
    case MACRO: return "macro";
    case NIL: return "nil";
    }
}

size_t obj_hash(tlisp_obj_t *obj)
{
    switch (obj->tag) {
    case BOOL:
    case NUM:
    case STRING:
    case SYMBOL:
    case CONS:
    case DICT:
    case VEC:
    case NFUNC:
    case LAMBDA:
    case MACRO:
    case NIL:
        break;
    }
    return 0;                   /* TODO: IMPL */
}

int obj_equals(tlisp_obj_t *first, tlisp_obj_t *second)
{
    if (first->tag != second->tag) {
        return 0;
    }
    switch (first->tag) {
    case NUM:
        return first->num == second->num;
    case STRING:
        return !strcmp(first->str, second->str);
    case SYMBOL:
        return !strcmp(first->sym, second->sym);
    case NFUNC:
        return first->fn == second->fn;
    case BOOL:
    case CONS:
    case DICT:
    case VEC:
    case LAMBDA:
    case MACRO:
    case NIL:
        return first == second;
    }
}

static
void cons_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
{
#define REMAINING (maxlen - (end - str))
    char *end = str;

    if (maxlen < 3) return;

    *end++ = '(';
    while (obj && REMAINING > 2) {
        obj_nstr(obj->car, end, REMAINING - 2);
        while (*end && REMAINING > 2) {
            end++;
        }
        obj = obj->cdr;
        if (obj && REMAINING > 2) {
            *end++ = ' ';
        }
    }
    *end++ = ')';
    end[0] = 0;
#undef REMAINING    
}

struct dict_str_state {
    char *start;
    char *end;
    int maxlen;
    int nvisited;
    int dictlen;
};

static
void dict_str_visitor(tlisp_obj_t *key, tlisp_obj_t *val, void *stateptr)
{
#define REMAINING (state->maxlen - (state->end - state->start))
    struct dict_str_state *state = (struct dict_str_state *)stateptr;
    int unseen;

    if (REMAINING <= 2) return;
    
    obj_nstr(key, state->end, REMAINING - 2);
    while (*state->end && REMAINING > 2) {
        state->end++;
    }

    if (REMAINING <= 3) return;
    
    *state->end++ = ' ';
    obj_nstr(val, state->end, REMAINING - 2);
    while (*state->end && REMAINING > 2) {
        state->end++;
    }
    state->nvisited++;
    unseen = state->dictlen - state->nvisited;
    if (REMAINING > 2 && unseen > 0) {
        *state->end++ = ' ';
    }
#undef REMAINING
}

static
void dict_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
{
    struct dict_str_state state = {
        .start = str,
        .end = str,
        .maxlen = maxlen,
        .nvisited = 0,
        .dictlen = obj->dict.len
    };
    
    if (maxlen < 4) return;
    
    *state.end++ = '#';
    *state.end++ = '(';
    dict_for_each(&obj->dict, dict_str_visitor, &state);
    *state.end++ = ')';
    state.end[0] = 0;
}

static
void vec_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
{
#define REMAINING (maxlen - (end - str))
    char *end = str;
    int i;

    if (maxlen < 3) return;
    
    *end++ = '[';
    for (i = 0; i < obj->vec.len && REMAINING > 2; i++) {
        obj_nstr(obj->vec.elems[i], end, REMAINING - 2);
        while (*end && REMAINING > 2) {
            end++;
        }
        if (i < obj->vec.len - 1 && REMAINING > 2) {
            *end++ = ' ';
        }
    }
    *end++ = ']';
    end[0] = 0;
#undef REMAINING
}

char *obj_nstr(tlisp_obj_t *obj, char *str, size_t maxlen)
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
    case CONS: 
        cons_nstr(obj, str, maxlen);
        break;
    case DICT:
        dict_nstr(obj, str, maxlen);
        break;
    case VEC:
        vec_nstr(obj, str, maxlen);
        break;
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
    return str;
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

void line_info_add(line_info_t *info, tlisp_obj_t *obj,
                   int start_line, int end_line)
{
    line_info_entry_t *entry = malloc(sizeof(line_info_entry_t));

    entry->obj = obj;
    entry->start_line = start_line;
    entry->end_line = end_line;
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
void print_lines(line_info_t *info, int start, int end)
{
    char *cursor = info->text;
    int curr = 1;

    while (curr < start) {
        cursor = strchr(cursor, '\n');
        cursor++;
        curr++;
    }
    while (*cursor && curr <= end) {
        fprintf(stderr, "%c", *cursor);
        if (*cursor == '\n') {
            curr++;
        }
        cursor++;
    }
    fflush(stderr);
}

void line_info_print(line_info_t *info, tlisp_obj_t *obj)
{
    line_info_entry_t *entry = find_entry(info, obj);

    if (!entry) return;
    if (entry->start_line == entry->end_line) {
        fprintf(stderr, "Line %d\n", entry->start_line);
    } else {
        fprintf(stderr, "Lines %d-%d\n", entry->start_line, entry->end_line);
    }
    print_lines(info, entry->start_line, entry->end_line);
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
    line_info_add(&source->line_info, expr, start_line, end_line);
    source->expressions[source->nexpressions] = expr;
    source->nexpressions++;
}
