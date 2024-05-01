#include "map.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// Increasing the table size dynamically is not yet supported.
static const int HASH_TABLE_SIZE = 256;

#define CPY(dst, dst_ind, src, src_ind, item_size) \
    memcpy((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define CMP(dst, dst_ind, src, src_ind, item_size) \
    memcmp((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define AT(v, ind, item_size) \
    ((void*)(v) + (item_size) * (ind))

struct chan_hash_map {
    struct chan_map map;
    size_t key_size;
    size_t value_size;
    // Number of keys.
    size_t size;
    // Number of slots.
    size_t capacity;
    void *key_data;
    void *value_data;
    int *hash_to_key_ind;
    size_t (*hasher)(void*);
};

// Returns position in the hash table where the key was found. In absence of
// collisions, the return value is same as the `hash` argument. If the key was
// not found, returns -1 and sets `new_key_ind` to the position where new one
// should be inserted.
static int
find_key_ind(const struct chan_map *map, void *key, size_t hash, int *new_key_ind)
{
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        size_t ind = (hash + i) % HASH_TABLE_SIZE;
        int key_ind = v->hash_to_key_ind[ind];
        if (key_ind == -1) {
            if (new_key_ind) *new_key_ind = ind;
            return -1;
        }
        if (CMP(v->key_data, key_ind, key, 0, v->key_size) == 0) {
            return key_ind;
        }
    }
    assert(false && "unexpected: hash map is full");
    *new_key_ind = -1;
    return -1;
}

static void
chan_hash_map_clear(struct chan_map *map)
{
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) v->hash_to_key_ind[i] = -1;
    v->size = 0;
}

static size_t
chan_hash_map_size(const struct chan_map *map)
{
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    return v->size;
}

static void*
chan_hash_map_at(const struct chan_map *map, void *key)
{
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    if (v->size == 0) return NULL;
    const size_t hash = v->hasher(key) % HASH_TABLE_SIZE;
    const int i = find_key_ind(map, key, hash, NULL);
    return i >= 0 ? AT(v->value_data, i, v->value_size) : NULL;
}

static void
chan_hash_map_remove(struct chan_map *map, void *key)
{
    assert(false && "not implemented");
}

static void
chan_hash_map_insert(struct chan_map *map, void *key, void *value)
{
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    if (v->capacity == 0) {
        v->hash_to_key_ind = realloc(v->hash_to_key_ind, HASH_TABLE_SIZE * sizeof(v->key_size));
        for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) v->hash_to_key_ind[i] = -1;
    }

    const size_t hash = v->hasher(key) % HASH_TABLE_SIZE;
    int new_key_ind;
    const int key_ind = find_key_ind(map, key, hash, &new_key_ind);
    if (key_ind == -1) {
        // New key.
        if (v->size >= v->capacity) {
            const size_t n = v->size == 0 ? 4 : 3 * v->size / 2;
            v->key_data = realloc(v->key_data, n * sizeof(v->key_size));
            v->value_data = realloc(v->value_data, n * sizeof(v->value_size));
            v->capacity = n;
        }
        CPY(v->key_data, v->size, key, 0, v->key_size);
        CPY(v->value_data, v->size, value, 0, v->value_size);
        v->hash_to_key_ind[new_key_ind] = v->size;
        v->size++;
    }
    else {
        // Replace existing key.
        CPY(v->key_data, key_ind, key, 0, v->key_size);
        CPY(v->value_data, key_ind, value, 0, v->value_size);
    }
}

static struct chan_map_iter
chan_hash_map_iter_new(const struct chan_map *map)
{
    struct chan_map_iter map_iter;
    return map_iter;
}

static struct chan_map_iter_item*
chan_hash_map_iter_next(const struct chan_map *map, struct chan_map_iter *map_iter)
{
    return NULL;
}

static void
chan_hash_map_debug_print(
    const struct chan_map *map,
    int (*print_key)(char *dest, int n, void *a),
    int (*print_value)(char *dest, int n, void *a)
) {
    struct chan_hash_map *v = (struct chan_hash_map*)map;
    const int bufSize = 256;
    char buf0[bufSize];
    char buf1[bufSize];
    printf("size %zu, capacity %zu\n", v->size, v->capacity);
    printf("hash table ind -> key ind:\n");
    for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        const int key_ind = v->hash_to_key_ind[i];
        if (key_ind < 0) continue;
        printf("* %zu -> %d\n", i, key_ind);
    }
    printf("key -> value:\n");
    for (size_t i = 0; i < v->size; ++i) {
        print_key(buf0, bufSize, AT(v->key_data, i, v->key_size));
        print_value(buf1, bufSize, AT(v->value_data, i, v->value_size));
        printf("* %s -> %s\n", buf0, buf1);
    }
}

static void
chan_hash_map_free(struct chan_map *map)
{
    assert(map);
    chan_hash_map_clear(map);

    struct chan_hash_map *v = (struct chan_hash_map*)map;
    if (v->key_data) free(v->key_data);
    if (v->value_data) free(v->value_data);
    if (v->hash_to_key_ind) free(v->hash_to_key_ind);
    free(v);
}

struct chan_map*
chan_hash_map_new(
    size_t key_size,
    size_t value_size,
    size_t (*hasher)(void*)
) {
    static const struct chan_map_vtable vtable = {
        chan_hash_map_free,
        chan_hash_map_clear,
        chan_hash_map_size,
        chan_hash_map_insert,
        chan_hash_map_at,
        chan_hash_map_remove,
        chan_hash_map_iter_new,
        chan_hash_map_iter_next,
        chan_hash_map_debug_print,
    };
    static struct chan_map map = { &vtable };
    struct chan_hash_map *hash_map = malloc(sizeof(*hash_map));
    memcpy(&hash_map->map, &map, sizeof(map));

    hash_map->key_size = key_size;
    hash_map->value_size = value_size;
    hash_map->size = 0;
    hash_map->capacity = 0;
    hash_map->key_data = NULL;
    hash_map->value_data = NULL;
    hash_map->hash_to_key_ind = NULL;
    hash_map->hasher = hasher;

    return &hash_map->map;
}
