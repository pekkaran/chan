#include "map.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CPY(dst, dst_ind, src, src_ind, item_size) \
    memcpy((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define CMP(dst, dst_ind, src, src_ind, item_size) \
    memcmp((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define AT(v, ind, item_size) \
    ((void*)(v) + (item_size) * (ind))

struct chan_naive_map {
    struct chan_map map;
    size_t key_size;
    size_t value_size;
    size_t size;
    size_t capacity;
    void *key_data;
    void *value_data;
};

static void
chan_naive_map_clear(struct chan_map *map)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    v->size = 0;
}

static size_t
chan_naive_map_size(const struct chan_map *map)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    return v->size;
}

static int
chan_naive_map_index(const struct chan_map *map, void *key)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    for (size_t i = 0; i < v->size; ++i) {
        if (CMP(v->key_data, i, key, 0, v->key_size) == 0) {
            return i;
        }
    }
    return -1;
}

static void*
chan_naive_map_at(const struct chan_map *map, void *key)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    const int i = chan_naive_map_index(map, key);
    return i >= 0 ? AT(v->value_data, i, v->value_size) : NULL;
}

static void
chan_naive_map_remove(struct chan_map *map, void *key)
{
    const int n = chan_naive_map_index(map, key);
    assert(n >= 0);
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    v->size--;
    for (size_t i = n; i < v->size; ++i) {
        CPY(v->key_data, i, v->key_data, i + 1, v->key_size);
        CPY(v->value_data, i, v->value_data, i + 1, v->value_size);
    }
}

static void
chan_naive_map_insert(struct chan_map *map, void *key, void *value)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    if (chan_naive_map_at(map, key) == NULL) {
        // New key.
        if (v->size >= v->capacity) {
            const size_t n = v->size == 0 ? 4 : 3 * v->size / 2;
            v->key_data = realloc(v->key_data, n * sizeof(v->key_size));
            v->value_data = realloc(v->value_data, n * sizeof(v->value_size));
            v->capacity = n;
        }
        CPY(v->key_data, v->size, key, 0, v->key_size);
        CPY(v->value_data, v->size, value, 0, v->value_size);
        v->size++;
    }
    else {
        // Replace existing key.
        const int i = chan_naive_map_index(map, key);
        CPY(v->key_data, i, key, 0, v->key_size);
        CPY(v->value_data, i, value, 0, v->value_size);
    }
}

static struct chan_map_iter
chan_naive_map_iter_new(const struct chan_map *map)
{
    struct chan_map_iter map_iter;
    map_iter.ind = 0;
    return map_iter;
}

static struct chan_map_iter_item*
chan_naive_map_iter_next(const struct chan_map *map, struct chan_map_iter *map_iter)
{
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    if (map_iter->ind >= v->size) return NULL;
    map_iter->map_iter_item.key = AT(v->key_data, map_iter->ind, v->key_size);
    map_iter->map_iter_item.value = AT(v->value_data, map_iter->ind, v->value_size);
    map_iter->ind++;
    return &map_iter->map_iter_item;
}

static void
chan_naive_map_debug_print(
    const struct chan_map *map,
    int (*print_key)(char *dest, int n, void *a),
    int (*print_value)(char *dest, int n, void *a)
) {
    struct chan_naive_map *v = (struct chan_naive_map*)map;
    const int bufSize = 256;
    char buf0[bufSize];
    char buf1[bufSize];
    printf("size %zu, capacity %zu\n", v->size, v->capacity);
    for (size_t i = 0; i < v->size; ++i) {
        print_key(buf0, bufSize, AT(v->key_data, i, v->key_size));
        print_value(buf1, bufSize, AT(v->value_data, i, v->value_size));
        printf("* %s -> %s\n", buf0, buf1);
    }
}

static void
chan_naive_map_free(struct chan_map *map)
{
    assert(map);
    chan_naive_map_clear(map);

    struct chan_naive_map *v = (struct chan_naive_map*)map;
    if (v->key_data) free(v->key_data);
    if (v->value_data) free(v->value_data);
    free(v);
}

struct chan_map*
chan_naive_map_new(size_t key_size, size_t value_size)
{
    static const struct chan_map_vtable vtable = {
        chan_naive_map_free,
        chan_naive_map_clear,
        chan_naive_map_size,
        chan_naive_map_insert,
        chan_naive_map_at,
        chan_naive_map_remove,
        chan_naive_map_iter_new,
        chan_naive_map_iter_next,
        chan_naive_map_debug_print,
    };
    static struct chan_map map = { &vtable };
    struct chan_naive_map *naive_map = malloc(sizeof(*naive_map));
    memcpy(&naive_map->map, &map, sizeof(map));

    naive_map->key_size = key_size;
    naive_map->value_size = value_size;
    naive_map->size = 0;
    naive_map->capacity = 0;
    naive_map->key_data = NULL;
    naive_map->value_data = NULL;

    return &naive_map->map;
}
