
#include "dict.h"
#include <stdlib.h>

#define MIN_CAP 8

tlisp_dict_t *dict_new(void)
{
    tlisp_dict_t *dict = malloc(sizeof(tlisp_dict_t));
    dict->len = 0;
    dict->cap = MIN_CAP;
    dict->entries = calloc(dict->cap, sizeof(tlisp_dict_entry_t));
    return dict;
}

void dict_free(tlisp_dict_t *dict)
{
    free(dict->entries);
    free(dict);
}

static
void dict_resize(tlisp_dict_t *dict, int cap)
{
    tlisp_dict_entry_t *entries = dict->entries;
    size_t old_cap = dict->cap;
    int i;

    dict->cap = cap;
    dict->entries = calloc(dict->cap, sizeof(tlisp_dict_entry_t));
    for (i = 0; i < old_cap; i++) {
        if (entries[i].valid) {
            dict_ins(dict, entries[i].key, entries[i].val);
        }
    }
    free(entries);
}

tlisp_obj_t *dict_ins(tlisp_dict_t *dict, tlisp_obj_t *key, tlisp_obj_t *val)
{
    size_t hash = obj_hash(key); 
    size_t idx = hash % dict->cap;

    if (dict->len >= (dict->cap * 3) / 4) {
        dict_resize(dict, dict->cap * 2);
        idx = hash % dict->cap;
    }
    while (dict->entries[idx].key && dict->entries[idx].valid) {
        if (obj_equals(dict->entries[idx].key, key)) {
            tlisp_obj_t *old = dict->entries[idx].val;
            dict->entries[idx].val = val;
            return old;
        }
        idx = (idx + 1) % dict->cap;
    }
    dict->entries[idx].key = key;
    dict->entries[idx].val = val;
    dict->entries[idx].valid = 1;
    dict->len++;
    return NULL;
}

static
tlisp_dict_entry_t *dict_get_internal(tlisp_dict_t *dict, tlisp_obj_t *key)
{
    size_t idx = obj_hash(key) % dict->cap;

     while (dict->entries[idx].key) {
        if (dict->entries[idx].valid &&
            obj_equals(dict->entries[idx].key, key)) {
            
            return dict->entries + idx;
        }
        idx = (idx + 1) % dict->cap;
     }
     return NULL;
}


tlisp_obj_t *dict_get(tlisp_dict_t *dict, tlisp_obj_t *key)
{
    tlisp_dict_entry_t *entry = dict_get_internal(dict, key);
    return entry ? entry->val : NULL;
}

tlisp_obj_t *dict_rem(tlisp_dict_t *dict, tlisp_obj_t *key)
{
    tlisp_dict_entry_t *entry = dict_get_internal(dict, key);

    if (!entry){
        return NULL;
    }
    entry->valid = 0;
    dict->len--;
    if (dict->cap >= 2 * MIN_CAP && dict->len < dict->cap / 4) {
        dict_resize(dict, dict->cap / 2);
    }
    return entry->val;
}

int dict_len(tlisp_dict_t *dict)
{
    return dict->len;
}

void dict_for_each(tlisp_dict_t *dict, dict_visitor fn, void *state)
{
    int i;

    for (i = 0; i < dict->cap; i++) {
        if (dict->entries[i].valid) {
            fn(dict->entries[i].key, dict->entries[i].val, state);
        }
    }
}
