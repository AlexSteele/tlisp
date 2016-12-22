
#include "read.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

typedef struct read_state {
    int line;
    int col;
    char *cursor; 
} read_state;

void reader_init(read_state *reader, char *source)
{
    reader->line = 1;
    reader->col = 1;
    reader->cursor = source;
}

void reader_adv(read_state *reader)
{
    if (*reader->cursor == '\n') {
        reader->line++;
        reader->col = 0;
    }
    reader->cursor++;
    reader->col++;
}

void reader_adv_n(read_state *reader, int n)
{
    reader->cursor += n;
}

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

tlisp_obj_t *read_list(read_state *reader)
{
    tlisp_obj_t *head = NULL;
    tlisp_obj_t *curr, *next;
    char c;

    reader_adv(reader);
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
    }
    *n = len;
    return forms;
}

