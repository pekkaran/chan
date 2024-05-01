#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CPY(dst, dst_ind, src, src_ind, value_size) \
    memcpy((void*)(dst) + (value_size) * (dst_ind), (void*)(src) + (value_size) * (src_ind), value_size);

#define AT(v, ind, item_size) \
    ((void*)(v) + (item_size) * (ind))

struct value_node {
    int neighbors[2]; // First is the previous, second the next node.
};

struct chan_linked_list {
    struct chan_list list;
    size_t capacity;
    size_t size;
    void *data;
    int value_size;
    struct value_node *value_nodes;
};

void
chan_linked_list_free(struct chan_list *list) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    assert(v);
    v->capacity = 0;
    v->size = 0;
    v->value_size = 0;
    if (v->data) free(v->data);
    if (v->data) free(v->value_nodes);
    free(v);
}

void*
chan_linked_list_at(const struct chan_list *list, size_t i) {
    assert(false && "not implemented");
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    return v->data + v->value_size * i;
}

size_t
chan_linked_list_size(const struct chan_list *list) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    return v->size;
}

size_t
chan_linked_list_capacity(const struct chan_list *list) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    return v->capacity;
}

void
chan_linked_list_reserve(struct chan_list *list, size_t capacity) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    if (capacity == 0) return;
    if (v->capacity >= capacity) return;
    v->data = realloc(v->data, capacity * sizeof(v->value_size));
    v->value_nodes = realloc(v->value_nodes, capacity * sizeof(*v->value_nodes));
    v->capacity = capacity;
    assert(v->data);
}

void
chan_linked_list_push(struct chan_list *list, void *value) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    if (v->size >= v->capacity) {
        const size_t capacity = v->size == 0 ? 4 : 3 * v->size / 2;
        chan_linked_list_reserve(list, capacity);
    }
    CPY(v->data, v->size, value, 0, v->value_size);
    v->size++;
    // TODO Setup value_nodes.
}

void
chan_linked_list_pop(struct chan_list *list) {
    assert(false && "not implemented");
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    assert(v->size > 0);
    if (v->size > 0) v->size--;
}

void
chan_linked_list_clear(struct chan_list *list) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    v->size = 0;
}

void
chan_linked_list_remove(struct chan_list *list, size_t n) {
    assert(false && "not implemented");
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    assert(n < v->size);
    v->size--;
    for (size_t i = n; i < v->size; ++i) {
        CPY(v->data, i, v->data, i + 1, v->value_size);
    }
}

void
chan_linked_list_insert(struct chan_list *list, size_t n, void *value) {
    assert(false && "not implemented");
    /* struct chan_linked_list *v = (struct chan_linked_list*)list; */
    /* v->size++; */
    /* assert(n < v->size); */
    /* chan_linked_list_reserve(list, v->size); */
    /* for (size_t i = v->size - 1; i > n; --i) { */
    /*     CPY(v->data, i, v->data, i - 1, v->value_size); */
    /* } */
    /* CPY(v->data, n, value, 0, v->value_size); */
}

void
chan_linked_list_resize(struct chan_list *list, size_t n, void *value) {
    assert(false && "not implemented");
    /* struct chan_linked_list *v = (struct chan_linked_list*)list; */
    /* const size_t n0 = v->size; */
    /* chan_linked_list_reserve(list, n); */
    /* v->size = n; */
    /* for (size_t i = n0; i < n; ++i) { */
    /*     CPY(v->data, i, value, 0, v->value_size); */
    /* } */
}

static struct chan_list_iter
chan_linked_list_iter_new(const struct chan_list *list)
{
    struct chan_list_iter list_iter;
    list_iter.ind = 0;
    return list_iter;
}

static struct chan_list_iter_item*
chan_linked_list_iter_next(const struct chan_list *list, struct chan_list_iter *list_iter)
{
    assert(false && "not implemented");
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    if (list_iter->ind >= v->size) return NULL;
    list_iter->list_iter_item.value = AT(v->data, list_iter->ind, v->value_size);
    list_iter->ind++;
    return &list_iter->list_iter_item;
}

static void
chan_linked_list_debug_print(
    const struct chan_list *list,
    int (*print_value)(char *dest, int n, void *a)
) {
    struct chan_linked_list *v = (struct chan_linked_list*)list;
    const int bufSize = 256;
    char buf[bufSize];
    printf("size %zu, capacity %zu [", v->size, v->capacity);
    for (size_t i = 0; i < v->size; ++i) {
        if (i > 0) printf(", ");
        print_value(buf, bufSize, AT(v->data, i, v->value_size));
        printf("%s", buf);
    }
    printf("]\n");
}

struct chan_list*
chan_linked_list_new(size_t value_size)
{
    static const struct chan_list_vtable vtable = {
        chan_linked_list_free,
        chan_linked_list_clear,
        chan_linked_list_size,
        chan_linked_list_insert,
        chan_linked_list_push,
        chan_linked_list_pop,
        chan_linked_list_at,
        chan_linked_list_remove,
        chan_linked_list_resize,
        chan_linked_list_iter_new,
        chan_linked_list_iter_next,
        chan_linked_list_debug_print,
    };
    static struct chan_list list = { &vtable };
    struct chan_linked_list *linked_list = malloc(sizeof(*linked_list));
    memcpy(&linked_list->list, &list, sizeof(list));

    linked_list->value_size = value_size;
    linked_list->size = 0;
    linked_list->capacity = 0;
    linked_list->data = NULL;

    return &linked_list->list;
}
