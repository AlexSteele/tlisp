
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
int numstart(char c)
{
    return isdigit(c) || c == '-';
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
    int in_comment;
} read_state;

static void reader_adv(read_state *);

static
void reader_init(read_state *reader, char *source)
{
    reader->line = 1;
    reader->col = 1;
    reader->cursor = source;
    reader->in_comment = 0;
    copy_line(source, reader->curr_line, MAX_LINE);
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
void read_fail(read_state *reader)
{
    char pos_str[256];
    if (*reader->cursor) {
        fprintf(stderr, "ERROR: Unexpected symbol '%c'.\n%s\n",
                *reader->cursor, reader_pos_str(reader, pos_str, 256));
    } else {
        fprintf(stderr, "ERROR: Unexpected EOF.\n%s\n",
                reader_pos_str(reader, pos_str, 256));
    }
    exit(1);
}

static
void reader_adv_comment(read_state *reader)
{
    reader->in_comment = 1;
    while (*reader->cursor) {
        reader_adv(reader);
        if (*reader->cursor == '\n') {
            reader_adv(reader);
            break;
        }
    }
    reader->in_comment = 0;
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
    if (!reader->in_comment && *reader->cursor == ';') {
        reader_adv_comment(reader);
    }
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
tlisp_obj_t *read_num(read_state *reader)
{
    tlisp_obj_t *obj = new_num();
    int neg = 0;
    int num = 0;
    char c;

    if (*reader->cursor == '-') {
        neg = 1;
        reader_adv(reader);
    }
    while ((c = *reader->cursor) && isdigit(c)) {
        num *= 10;
        num += c - '0';
        reader_adv(reader);
    }
    num = neg ? -num : num;
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

static tlisp_obj_t *read_literal(read_state *);

static
tlisp_obj_t *read_delimited_form(read_state *reader, char delim)
{
    tlisp_obj_t *head = NULL;
    tlisp_obj_t *curr;
    char c;

    reader_adv(reader);
    while (1) {
        c = *reader->cursor;
        if (!c) read_fail(reader);
        if (c == delim) break;
        if (whitespace(c)) {
            reader_adv(reader);
        } else {
            tlisp_obj_t *next = new_cons();
            next->car = read_literal(reader);
            if (head) {
                curr->cdr = next;
            } else {
                head = next;
            }
            curr = next;            
        }
    }
    reader_adv(reader);
    return head;
}

static
tlisp_obj_t *read_list_literal(read_state *reader)
{
    tlisp_obj_t *obj = read_delimited_form(reader, ')');
    
    if (!obj) return tlisp_nil;
    
    return obj;
}

static
tlisp_obj_t *read_dict_literal(read_state *reader)
{
    tlisp_obj_t *obj = new_cons();

    obj->car = tlisp_hashtag;
    reader_adv(reader);
    if (*reader->cursor != '(') {
        read_fail(reader);
    }
    obj->cdr = read_delimited_form(reader, ')');
    return obj;
}

static
tlisp_obj_t *read_vec_literal(read_state *reader)
{
    tlisp_obj_t *obj = new_cons();
    
    obj->car = tlisp_bracket;
    obj->cdr = read_delimited_form(reader, ']');
    return obj;
}

static
tlisp_obj_t *read_quoted_literal(read_state *reader)
{
    tlisp_obj_t *obj = new_cons();

    obj->car = tlisp_quote;
    reader_adv(reader);
    obj->cdr = *reader->cursor == '(' ?
        read_list_literal(reader) : read_sym(reader);
    return obj;
}

static
tlisp_obj_t *read_bquoted_literal(read_state *reader)
{
    tlisp_obj_t *obj = new_cons();

    obj->car = tlisp_backquote;
    reader_adv(reader);
    obj->cdr = *reader->cursor == '(' ?
        read_list_literal(reader) : read_sym(reader);
    return obj;
}

static
tlisp_obj_t *read_literal(read_state *reader)
{
    char c = *reader->cursor;

    if (c == ')' || c == ']') read_fail(reader);

    if (c == '"') {
        return read_str(reader);
    } else if (numstart(c)) {
        return read_num(reader);
    } else if (c == '(') {
        return read_list_literal(reader);
    } else if (c == '\'') {
        return read_quoted_literal(reader);
    } else if (c == '`') {
        return read_bquoted_literal(reader);
    } else if (c == '#') {
        return read_dict_literal(reader);
    } else if (c == '[') {
        return read_vec_literal(reader);
    } else {
        return read_sym(reader);
    }
}

source_t read(char *text)
{
    source_t source;
    read_state reader;
    char c;
    
    source_init(&source, text);
    reader_init(&reader, text);
    
    while ((c = *reader.cursor)) {
        if (whitespace(c)) {
            reader_adv(&reader);
        } else if (c == ';') {
            reader_adv_comment(&reader);
        } else {
            int start_line = reader.line;
            tlisp_obj_t *expression = read_literal(&reader);
            int end_line = reader.line;
            source_add_expr(&source, expression, start_line, end_line);
        } 
    }
    return source;
}
