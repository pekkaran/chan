#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct chan_map {
    const struct chan_map_vtable * const vtable;
};

// Value from the map iterator. Iterator is finished if key == NULL.
struct chan_map_iter_item {
    void *key;
    void *value;
};

// Iterator status.
struct chan_map_iter {
    size_t ind;
    // Storing the yielded value here allows returning it as pointer and
    // making the API for iterating in a loop nicer.
    struct chan_map_iter_item map_iter_item;
};

struct chan_map_vtable {
    void (*free)(struct chan_map*);
    void (*clear)(struct chan_map*);
    size_t (*size)(const struct chan_map*);
    void (*insert)(struct chan_map*, void*, void*);
    void* (*at)(const struct chan_map*, void*);
    void (*remove)(struct chan_map*, void*);
    struct chan_map_iter (*iter_new)(const struct chan_map*);
    struct chan_map_iter_item* (*iter_next)(const struct chan_map*, struct chan_map_iter*);
    void (*debug_print)(
        const struct chan_map*,
        int (*print_key)(char *dest, int n, void *a),
        int (*print_value)(char *dest, int n, void *a)
    );
};

void chan_map_free(struct chan_map *s);
void chan_map_clear(struct chan_map *s);
size_t chan_map_size(const struct chan_map *s);
void chan_map_insert(struct chan_map *s, void *key, void *value);
void* chan_map_at(const struct chan_map *s, void *key);
void chan_map_remove(struct chan_map *s, void *key);
struct chan_map_iter chan_map_iter_new(const struct chan_map*);
struct chan_map_iter_item* chan_map_iter_next(const struct chan_map*, struct chan_map_iter*);
void chan_map_debug_print(
    const struct chan_map *s,
    int (*print_key)(char *dest, int n, void *a),
    int (*print_value)(char *dest, int n, void *a)
);

// The simplest possible implementation.
// Uses just two arrays and performs searches in O(n).
struct chan_map *chan_naive_map_new(size_t key_size, size_t value_size);

// Binary Search Tree
// Improves upon `naive` with `O(log n)` search.
struct chan_map *chan_bst_map_new(
    size_t key_size,
    size_t value_size,
    // Function that returns true iff first argument of key type is less than or equal to the
    // second argument.
    // TODO Allow omitting this and just use the byte data.
    bool (*less)(void*, void*)
);

struct chan_map *chan_hash_map_new(
    size_t key_size,
    size_t value_size,
    size_t (*hasher)(void*)
);
