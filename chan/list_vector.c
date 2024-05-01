#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CPY(dst, dst_ind, src, src_ind, value_size) \
    memcpy((void*)(dst) + (value_size) * (dst_ind), (void*)(src) + (value_size) * (src_ind), value_size);

#define AT(v, ind, item_size) \
    ((void*)(v) + (item_size) * (ind))

struct chan_vector_list {
    struct chan_list list;
    size_t capacity;
    size_t size;
    void *data;
    int value_size;
};

void
chan_vector_list_free(struct chan_list *list) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    assert(v);
    v->capacity = 0;
    v->size = 0;
    v->value_size = 0;
    if (v->data) free(v->data);
    free(v);
}

void*
chan_vector_list_at(const struct chan_list *list, size_t i) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    return v->data + v->value_size * i;
}

size_t
chan_vector_list_size(const struct chan_list *list) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    return v->size;
}

size_t
chan_vector_list_capacity(const struct chan_list *list) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    return v->capacity;
}

void
chan_vector_list_reserve(struct chan_list *list, size_t n) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    if (n == 0) return;
    if (v->capacity >= n) return;
    v->data = realloc(v->data, n * sizeof(v->value_size));
    v->capacity = n;
    assert(v->data);
}

void
chan_vector_list_push(struct chan_list *list, void *value) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    if (v->size >= v->capacity) {
        const size_t n = v->size == 0 ? 4 : 3 * v->size / 2;
        chan_vector_list_reserve(list, n);
    }
    CPY(v->data, v->size, value, 0, v->value_size);
    v->size++;
}

void
chan_vector_list_pop(struct chan_list *list) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    assert(v->size > 0);
    if (v->size > 0) v->size--;
}

void
chan_vector_list_clear(struct chan_list *list) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    v->size = 0;
}

void
chan_vector_list_remove(struct chan_list *list, size_t n) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    assert(n < v->size);
    v->size--;
    for (size_t i = n; i < v->size; ++i) {
        CPY(v->data, i, v->data, i + 1, v->value_size);
    }
}

void
chan_vector_list_insert(struct chan_list *list, size_t n, void *value) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    v->size++;
    assert(n < v->size);
    chan_vector_list_reserve(list, v->size);
    for (size_t i = v->size - 1; i > n; --i) {
        CPY(v->data, i, v->data, i - 1, v->value_size);
    }
    CPY(v->data, n, value, 0, v->value_size);
}

void
chan_vector_list_resize(struct chan_list *list, size_t n, void *value) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    const size_t n0 = v->size;
    chan_vector_list_reserve(list, n);
    v->size = n;
    for (size_t i = n0; i < n; ++i) {
        CPY(v->data, i, value, 0, v->value_size);
    }
}

static struct chan_list_iter
chan_vector_list_iter_new(const struct chan_list *list)
{
    struct chan_list_iter list_iter;
    list_iter.ind = 0;
    return list_iter;
}

static struct chan_list_iter_item*
chan_vector_list_iter_next(const struct chan_list *list, struct chan_list_iter *list_iter)
{
    struct chan_vector_list *v = (struct chan_vector_list*)list;
    if (list_iter->ind >= v->size) return NULL;
    list_iter->list_iter_item.value = AT(v->data, list_iter->ind, v->value_size);
    list_iter->ind++;
    return &list_iter->list_iter_item;
}

static void
chan_vector_list_debug_print(
    const struct chan_list *list,
    int (*print_value)(char *dest, int n, void *a)
) {
    struct chan_vector_list *v = (struct chan_vector_list*)list;
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
chan_vector_list_new(size_t value_size)
{
    static const struct chan_list_vtable vtable = {
        chan_vector_list_free,
        chan_vector_list_clear,
        chan_vector_list_size,
        chan_vector_list_insert,
        chan_vector_list_push,
        chan_vector_list_pop,
        chan_vector_list_at,
        chan_vector_list_remove,
        chan_vector_list_resize,
        chan_vector_list_iter_new,
        chan_vector_list_iter_next,
        chan_vector_list_debug_print,
    };
    static struct chan_list list = { &vtable };
    struct chan_vector_list *vector_list = malloc(sizeof(*vector_list));
    memcpy(&vector_list->list, &list, sizeof(list));

    vector_list->value_size = value_size;
    vector_list->size = 0;
    vector_list->capacity = 0;
    vector_list->data = NULL;

    return &vector_list->list;
}
