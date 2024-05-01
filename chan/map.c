#include "map.h"

void
chan_map_free(struct chan_map *s)
{
    s->vtable->free(s);
}

void
chan_map_clear(struct chan_map *s)
{
    s->vtable->clear(s);
}

size_t
chan_map_size(const struct chan_map *s)
{
    return s->vtable->size(s);
}

void
chan_map_insert(struct chan_map *s, void *key, void *value)
{
    s->vtable->insert(s, key, value);
}

void*
chan_map_at(const struct chan_map *s, void *key)
{
    return s->vtable->at(s, key);
}

void
chan_map_remove(struct chan_map *s, void *key)
{
    return s->vtable->remove(s, key);
}

struct chan_map_iter
chan_map_iter_new(const struct chan_map *s)
{
    return s->vtable->iter_new(s);
}

struct chan_map_iter_item*
chan_map_iter_next(const struct chan_map *s, struct chan_map_iter *iter)
{
    return s->vtable->iter_next(s, iter);
}

void
chan_map_debug_print(
    const struct chan_map *s,
    int (*print_key)(char *dest, int n, void *a),
    int (*print_value)(char *dest, int n, void *a)
) {
    s->vtable->debug_print(s, print_key, print_value);
}
