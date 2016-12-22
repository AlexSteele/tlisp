
#include "read.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
int whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static
void copy_line(char *src, char *dest, size_t maxlen)
{
    int i = 0;
    
    while (*src && i < maxlen - 1) {
        dest[i] = *src;
        if (*src == '\n' || *src == '\r') {
            break;
        }
        src++;
        i++;
    }
    dest[i] = 0;
}

#define MAX_LINE 256

typedef struct read_state {
    int line;
    int col;
    char curr_line[MAX_LINE];
    char *cursor;
} read_state;

static
void reader_init(read_state *reader, char *source)
{
    reader->line = 1;
    reader->col = 1;
    reader->cursor = source;
    copy_line(source, reader->curr_line, MAX_LINE);
}

static
void reader_adv(read_state *reader)
{
    if (*reader->cursor == '\n') {
        reader->line++;
        reader->col = 1;
        copy_line(reader->cursor + 1, reader->curr_line, MAX_LINE);
    } else {
        reader->col++;
    }
    reader->cursor++;
}

static
void reader_adv_n(read_state *reader, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        reader_adv(reader);
    }
}

static
char *reader_pos_str(read_state *reader, char *str, size_t maxlen)
{
    int len;
    int len_with_arrow;

    snprintf(str, maxlen, "Line %d Column %d\n%s\n",
             reader->line, reader->col, reader->curr_line);
    len = strlen(str);
    len_with_arrow = len + reader->col + 1;
    if (len_with_arrow <= maxlen) {
        char *end = str + len; 
        if (reader->col > 1) {
            memset(end, ' ', reader->col - 1);            
        }
        end[reader->col - 1] = '^';
        end[reader->col] = 0;
    }
    return str;
}

static
tlisp_obj_t *read_num(read_state *reader)
{
    tlisp_obj_t *obj = new_num();
    int num = 0;
    char c;

    while ((c = *reader->cursor) && !whitespace(c) && c != ')') {
        num *= 10;
        num += c - '0';
        reader_adv(reader);
    }
    obj->num = num;
    return obj;
}

static
tlisp_obj_t *read_str(read_state *reader)
{
    tlisp_obj_t *obj = new_str();
    char *lead;
    size_t len = 0;

    reader_adv(reader);
    lead = reader->cursor;
    while (*lead && *lead != '"') {
        len++;
        lead++;
    }
    obj->str = strndup(reader->cursor, len);
    reader_adv_n(reader, len + 1);
    return obj;
}

static
tlisp_obj_t *read_sym(read_state *reader)
{
    tlisp_obj_t *obj = new_sym();
    char *lead = reader->cursor;
    size_t len = 0;

    while (*lead && !whitespace(*lead) && *lead != ')') {
        len++;
        lead++;
    }
    obj->sym = strndup(reader->cursor, len);
    reader_adv_n(reader, len);
    return obj;
}

static
tlisp_obj_t *read_list(read_state *reader)
{
    tlisp_obj_t *head = NULL;
    tlisp_obj_t *curr, *next;
    char c;

    reader_adv(reader);
    if (*reader->cursor == ')') {
        reader_adv(reader);
        return tlisp_nil;
    }
    while ((c = *reader->cursor)) {
        if (whitespace(c)) {
            reader_adv(reader);
            continue;
        }
        if (c == ')') {
            reader_adv(reader);
            break;
        }

        next = new_cons();
        if (c == '"') {
            next->car = read_str(reader);
        } else if (isdigit(c)) {
            next->car = read_num(reader);
        } else if (c == '(') {
            next->car = read_list(reader);
        } else {
            next->car = read_sym(reader);
        }
        if (head) {
            curr->cdr = next;
        } else {
            head = next;
        }
        curr = next;
    }
    return head;
}

tlisp_obj_t **read(char *raw, size_t *n)
{
    size_t len = 0;
    size_t cap = 128;
    tlisp_obj_t **forms = malloc(sizeof(tlisp_obj_t*) * cap);
    read_state reader;
    char c;

    reader_init(&reader, raw);
    
    while ((c = *reader.cursor)) {
        if (whitespace(c)) {
            reader_adv(&reader);
        } else if (c == '(') {
            tlisp_obj_t *form = read_list(&reader);
            if (len == cap) {
                cap *= 2;
                forms = realloc(forms, sizeof(tlisp_obj_t*) * cap);
            }
            forms[len] = form;
            len++;
        } else {
            char pos_str[256];
            fprintf(stderr, "ERROR: Unexpected symbol '%c'.\n%s\n",
                    c, reader_pos_str(&reader, pos_str, 256));
            exit(1);
        }
    }
    *n = len;
    return forms;
}

