#include "list.h"

void
chan_list_free(struct chan_list *s)
{
    s->vtable->free(s);
}

void
chan_list_clear(struct chan_list *s)
{
    s->vtable->clear(s);
}

size_t
chan_list_size(const struct chan_list *s)
{
    return s->vtable->size(s);
}

void
chan_list_insert(struct chan_list *s, size_t ind, void *value)
{
    s->vtable->insert(s, ind, value);
}

void
chan_list_push(struct chan_list *s, void *value)
{
    s->vtable->push(s, value);
}

void
chan_list_pop(struct chan_list *s)
{
    s->vtable->pop(s);
}

void*
chan_list_at(const struct chan_list *s, size_t ind)
{
    return s->vtable->at(s, ind);
}

void
chan_list_remove(struct chan_list *s, size_t ind)
{
    return s->vtable->remove(s, ind);
}

void
chan_list_resize(struct chan_list *s, size_t ind, void *value)
{
    return s->vtable->resize(s, ind, value);
}

struct chan_list_iter
chan_list_iter_new(const struct chan_list *s)
{
    return s->vtable->iter_new(s);
}

struct chan_list_iter_item*
chan_list_iter_next(const struct chan_list *s, struct chan_list_iter *iter)
{
    return s->vtable->iter_next(s, iter);
}

void
chan_list_debug_print(
    const struct chan_list *s,
    int (*print_value)(char *dest, int n, void *a)
) {
    s->vtable->debug_print(s, print_value);
}
