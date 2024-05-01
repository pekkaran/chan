#include "map.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define CPY(dst, dst_ind, src, src_ind, item_size) \
    memcpy((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define CMP(dst, dst_ind, src, src_ind, item_size) \
    memcmp((void*)(dst) + (item_size) * (dst_ind), (void*)(src) + (item_size) * (src_ind), item_size)

#define AT(v, ind, item_size) \
    ((void*)(v) + (item_size) * (ind))

struct key_node {
    int children[2]; // -1 if none. Index 0 points to the smaller key, 1 == larger.
};

struct chan_bst_map {
    struct chan_map map;
    size_t key_size;
    size_t value_size;
    size_t size;
    size_t capacity;
    struct key_node *key_nodes;
    size_t *key_order;
    void *key_data;
    void *value_data;
    bool (*less)(void*, void*);
};

static void
build_key_order(const struct key_node *key_nodes, size_t *order_ind, size_t *key_order, int i)
{
    // Depth-first traversal.
    if (key_nodes[i].children[0] >= 0) build_key_order(key_nodes, order_ind, key_order, key_nodes[i].children[0]);
    key_order[(*order_ind)++] = i;
    if (key_nodes[i].children[1] >= 0) build_key_order(key_nodes, order_ind, key_order, key_nodes[i].children[1]);
}

static void
chan_bst_map_clear(struct chan_map *map)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    v->size = 0;
}

static size_t
chan_bst_map_size(const struct chan_map *map)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    return v->size;
}

static int
chan_bst_map_index(const struct chan_map *map, void *key)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    for (size_t i = 0; i < v->size; ++i) {
        if (CMP(v->key_data, i, key, 0, v->key_size) == 0) {
            return i;
        }
    }
    return -1;
}

static void*
chan_bst_map_at(const struct chan_map *map, void *key)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    if (v->size == 0) return NULL;
    int i = 0;
    while (i >= 0) {
        if (CMP(v->key_data, i, key, 0, v->key_size) == 0) {
            return AT(v->value_data, i, v->value_size);
        }
        if (v->less(key, AT(v->key_data, i, v->key_size))) i = v->key_nodes[i].children[0];
        else i = v->key_nodes[i].children[1];
    }
    return NULL;
}

static void
chan_bst_map_remove(struct chan_map *map, void *key)
{
    assert(false && "not implemented");
    // Code from map_naive.
    /*
    const int n = chan_bst_map_index(map, key);
    assert(n >= 0);
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    v->size--;
    for (size_t i = n; i < v->size; ++i) {
        CPY(v->key_data, i, v->key_data, i + 1, v->key_size);
        CPY(v->value_data, i, v->value_data, i + 1, v->value_size);
    }
    */
}

static void
chan_bst_map_insert(struct chan_map *map, void *key, void *value)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    if (chan_bst_map_at(map, key) == NULL) {
        // New key.
        if (v->size >= v->capacity) {
            const size_t n = v->size == 0 ? 4 : 3 * v->size / 2;
            v->key_nodes = realloc(v->key_nodes, n * sizeof(*v->key_nodes));
            v->key_order = realloc(v->key_order, n * sizeof(*v->key_order));
            v->key_data = realloc(v->key_data, n * sizeof(v->key_size));
            v->value_data = realloc(v->value_data, n * sizeof(v->value_size));
            v->capacity = n;
        }
        assert(v->value_data);
        CPY(v->key_data, v->size, key, 0, v->key_size);
        CPY(v->value_data, v->size, value, 0, v->value_size);
        v->key_nodes[v->size].children[0] = -1;
        v->key_nodes[v->size].children[1] = -1;
        // If existing tree, add as child to some node.
        if (v->size >= 1) {
            int i = 0;
            int valid_i = 0;
            bool isLess;
            while (i >= 0) {
                valid_i = i;
                isLess = v->less(key, AT(v->key_data, i, v->key_size));
                if (isLess) i = v->key_nodes[i].children[0];
                else i = v->key_nodes[i].children[1];
            }
            v->key_nodes[valid_i].children[!isLess] = v->size;
        }

        // Rebuild key order for iterators.
        //
        // Iterators could also maintain their own state to avoid this, but it
        // seems like either:
        // * The iterator would need to make an allocation to remember the path it has traversed,
        //   which is bad because then it requires a function call to free the memory.
        // * New "parent" field would be added to `key_node` so that some kind of simple state
        //   with few variables might allow traversing the tree(?).
        // * The data should be stored in sorted order to begin with, an then simple index would
        //   suffice as state.
        // * A fancy state would be need for a solution that "yields" the iterator elements by
        //   taking steps through the data together with the caller.
        size_t order_ind = 0;
        build_key_order(v->key_nodes, &order_ind, v->key_order, 0);

        v->size++;
        assert(order_ind == v->size);
    }
    else {
        // Replace existing key.
        const int i = chan_bst_map_index(map, key);
        CPY(v->key_data, i, key, 0, v->key_size);
        CPY(v->value_data, i, value, 0, v->value_size);
    }
}

static struct chan_map_iter
chan_bst_map_iter_new(const struct chan_map *map)
{
    struct chan_map_iter map_iter;
    map_iter.ind = 0;
    return map_iter;
}

static struct chan_map_iter_item*
chan_bst_map_iter_next(const struct chan_map *map, struct chan_map_iter *map_iter)
{
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    if (map_iter->ind >= v->size) return NULL;

    const size_t ind = v->key_order[map_iter->ind];
    map_iter->map_iter_item.key = AT(v->key_data, ind, v->key_size);
    map_iter->map_iter_item.value = AT(v->value_data, ind, v->value_size);
    map_iter->ind++;
    return &map_iter->map_iter_item;
}

static void
chan_bst_map_debug_print(
    const struct chan_map *map,
    int (*print_key)(char *dest, int n, void *a),
    int (*print_value)(char *dest, int n, void *a)
) {
    struct chan_bst_map *v = (struct chan_bst_map*)map;
    const int bufSize = 256;
    char buf0[bufSize];
    char buf1[bufSize];
    printf("size %zu, capacity %zu\n", v->size, v->capacity);
    for (size_t i = 0; i < v->size; ++i) {
        print_key(buf0, bufSize, AT(v->key_data, i, v->key_size));
        print_value(buf1, bufSize, AT(v->value_data, i, v->value_size));
        printf("* %s -> %s, node children: %d, %d\n", buf0, buf1,
            v->key_nodes[i].children[0], v->key_nodes[i].children[1]
        );
    }
}

static void
chan_bst_map_free(struct chan_map *map)
{
    assert(map);
    chan_bst_map_clear(map);

    struct chan_bst_map *v = (struct chan_bst_map*)map;
    if (v->key_nodes) free(v->key_nodes);
    if (v->key_data) free(v->key_data);
    if (v->value_data) free(v->value_data);
    free(v);
}

struct chan_map*
chan_bst_map_new(
    size_t key_size,
    size_t value_size,
    bool (*less)(void*, void*)
) {
    static const struct chan_map_vtable vtable = {
        chan_bst_map_free,
        chan_bst_map_clear,
        chan_bst_map_size,
        chan_bst_map_insert,
        chan_bst_map_at,
        chan_bst_map_remove,
        chan_bst_map_iter_new,
        chan_bst_map_iter_next,
        chan_bst_map_debug_print,
    };
    static struct chan_map map = { &vtable };
    struct chan_bst_map *bst_map = malloc(sizeof(*bst_map));
    memcpy(&bst_map->map, &map, sizeof(map));

    bst_map->key_size = key_size;
    bst_map->value_size = value_size;
    bst_map->size = 0;
    bst_map->capacity = 0;
    bst_map->key_nodes = NULL;
    bst_map->key_order = NULL;
    bst_map->key_data = NULL;
    bst_map->value_data = NULL;
    bst_map->less = less;

    return &bst_map->map;
}
