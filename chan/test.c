#include <chan/list.h>
#include <chan/map.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool less_int(void *a, void *b) { return *(int*)a <= *(int*)b; }
int print_int(char *dest, int n, void *a) { return snprintf(dest, n, "%d", *(int*)a); }
int print_float(char *dest, int n, void *a) { return snprintf(dest, n, "%.3f", *(float*)a); }
size_t hasher_int(void *a) { return (size_t)*(int*)a; }

// Purposefully bad hasher to test collision handling.
size_t bad_hasher_int(void *a) {
    const int v = *(int*)a;
    return (size_t)(10 + v / 5);
}

int
test_vector(int kind, bool print)
{
    struct chan_list *v;
    if (kind == 0) v = chan_vector_list_new(sizeof(int));
    else if (kind == 1) v = chan_linked_list_new(sizeof(int));
    else assert(false);

    const bool supports_resize = kind == 0;
    const bool supports_remove = kind == 0;

    printf("\n=== Testing vector kind %d\n", kind);

    for (int i = 0; i < 10; ++i) chan_list_push(v, &i);
    int item = 2;
    chan_list_push(v, &item);
    if (print) chan_list_debug_print(v, print_int);
    item = -8;
    if (supports_resize) chan_list_resize(v, 18, &item);
    if (print) chan_list_debug_print(v, print_int);
    item = -2;
    if (supports_resize) chan_list_resize(v, 10, &item);
    if (print) chan_list_debug_print(v, print_int);
    if (supports_remove) chan_list_remove(v, 2);
    if (print) chan_list_debug_print(v, print_int);
    if (supports_remove) chan_list_remove(v, 4);
    if (print) chan_list_debug_print(v, print_int);
    item = 10;
    chan_list_insert(v, 2, &item);
    if (print) chan_list_debug_print(v, print_int);
    item = 11;
    chan_list_insert(v, 0, &item);
    if (print) chan_list_debug_print(v, print_int);
    item = 12;
    chan_list_insert(v, 10, &item);
    if (print) chan_list_debug_print(v, print_int);

    chan_list_free(v);
    return 0;
}

int
test_map(int kind, bool print)
{
    printf("\n=== Testing map kind %d\n", kind);
    struct chan_map *map;
    if (kind == 0) map = chan_naive_map_new(sizeof(int), sizeof(float));
    else if (kind == 1) map = chan_bst_map_new(sizeof(int), sizeof(float), less_int);
    else if (kind == 2) map = chan_hash_map_new(sizeof(int), sizeof(float), bad_hasher_int);
    else assert(false);
    assert(map);

    assert(chan_map_size(map) == 0);
    int key = 8;
    float value = 5.67;
    assert(chan_map_at(map, &key) == NULL);
    chan_map_insert(map, &key, &value);
    assert(*(float*)chan_map_at(map, &key) == value);
    chan_map_clear(map);
    assert((float*)chan_map_at(map, &key) == NULL);

    assert(chan_map_size(map) == 0);
    int key0 = 3; float value0 = 4.;
    int key1 = -5; float value1 = 6.;
    int key2 = 2; float value2 = 8.;
    int key3 = -1; float value3 = 7.;
    int key4 = 9; float value4 = 1.;
    chan_map_insert(map, &key0, &value0);
    chan_map_insert(map, &key1, &value1);
    assert(chan_map_size(map) == 2);
    assert(*(float*)chan_map_at(map, &key0) == value0);
    assert(*(float*)chan_map_at(map, &key1) == value1);

    chan_map_insert(map, &key2, &value2);
    chan_map_insert(map, &key3, &value3);
    chan_map_insert(map, &key4, &value4);
    assert(chan_map_size(map) == 5);

    float value1_2 = 10.;
    chan_map_insert(map, &key1, &value1_2);
    assert(chan_map_size(map) == 5);
    assert(*(float*)chan_map_at(map, &key1) == value1_2);

    if (print) chan_map_debug_print(map, print_int, print_float);

    struct chan_map_iter it = chan_map_iter_new(map);
    struct chan_map_pair *map_pair;
    size_t i = 0;
    /* while ((map_pair = chan_map_iter_next(map, &it))) { */
    /*     printf("next %d -> %f\n", *(int*)map_pair->key, *(float*)map_pair->value); */
    /*     i++; */
    /* } */
    /* assert(i == 5); */

    if (kind != 1 && kind != 2) {
        chan_map_remove(map, &key0);
        assert(chan_map_size(map) == 4);
        chan_map_remove(map, &key1);
        assert(chan_map_size(map) == 3);
    }

    chan_map_free(map);
    return 0;
}

int
main()
{
    const bool print = true;
    if (test_vector(0, print)) return 1;
    /* if (test_vector(1, print)) return 1; */
    if (test_map(0, print)) return 1;
    if (test_map(1, print)) return 1;
    if (test_map(2, print)) return 1;
    return 0;
}
