#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct chan_list {
    const struct chan_list_vtable * const vtable;
};

// Value from the list iterator. Iterator is finished if value == NULL.
struct chan_list_iter_item {
    void *value;
    // TODO Some kind of handle that allows insertion and deletion,
    // in particular for linked list.
};

// Iterator status.
struct chan_list_iter {
    size_t ind;
    // Storing the yielded value here allows returning it as pointer and
    // making the API for iterating in a loop nicer.
    struct chan_list_iter_item list_iter_item;
};

struct chan_list_vtable {
    void (*free)(struct chan_list*);
    void (*clear)(struct chan_list*);
    size_t (*size)(const struct chan_list*);
    void (*insert)(struct chan_list*, size_t, void*);
    void (*push)(struct chan_list*, void*);
    void (*pop)(struct chan_list*);
    void* (*at)(const struct chan_list*, size_t);
    void (*remove)(struct chan_list*, size_t);
    void (*resize)(struct chan_list*, size_t, void*);
    struct chan_list_iter (*iter_new)(const struct chan_list*);
    struct chan_list_iter_item* (*iter_next)(const struct chan_list*, struct chan_list_iter*);
    void (*debug_print)(
        const struct chan_list*,
        int (*print_value)(char *dest, int n, void *a)
    );
};

void chan_list_free(struct chan_list *s);
void chan_list_clear(struct chan_list *s);
size_t chan_list_size(const struct chan_list *s);
void chan_list_insert(struct chan_list *s, size_t ind, void *value);
void chan_list_push(struct chan_list *s, void *value);
void chan_list_pop(struct chan_list *s);
void* chan_list_at(const struct chan_list *s, size_t ind);
void chan_list_remove(struct chan_list *s, size_t);
void chan_list_resize(struct chan_list *s, size_t, void*);
struct chan_list_iter chan_list_iter_new(const struct chan_list*);
struct chan_list_iter_item* chan_list_iter_next(const struct chan_list*, struct chan_list_iter*);
void chan_list_debug_print(
    const struct chan_list *s,
    int (*print_value)(char *dest, int n, void *a)
);

struct chan_list *chan_vector_list_new(size_t value_size);

struct chan_list *chan_linked_list_new(size_t value_size);
