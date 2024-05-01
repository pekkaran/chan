# C containers

C implementations of containers similar to C++ `std::vector`, `std::map`, and `std::unordered_map`. I made these from scratch to practice programming C and organizing such code. Obviously not something you should use in production, but could be helpful eg in LeetCode-like puzzles.

To the extent I have tested them, the implementations are "correct" and "useful", in the following sense:

* The container methods produce correct results,
* The keys and values can have arbitrary types.
* The standard operations such as insertions and deletions have appropriate computation complexities.
* The containers provide iterator methods.
* The API provides basic encapsulation to prevent user errors.
* Multiple implementations for each container type can be used via dynamic dispatch.
* The containers do not allocate unnecessary memory. More specifically, all the implementations store the values in a single continuous block of memory, unlike many implementations of C++ `std::map` and `std::unordered_map`.

## Build and test

Other than CMake and a C compiler there are no dependencies. Do the usual CMake routine at the repository root:

```bash
mkdir target
cd target
cmake ..
make
./chan_test
```

## The containers

### [list.h](chan/list.h) (C++ `std::vector`, `std::list`)

Interface for a vector or list. Implementations:

* [list_vector.c](chan/list_vector.c): Similar to C++ `std::vector`.
  * Insertion/deletion at the end is `O(1)`, in the middle `O(n)`.
* [list_linked.c](chan/list_linked.c): Similar to C++ `std::list`. (not fully implemented)

### [map.h](chan/map.h) (C++ `std::map`, `std::unordered_map`)

Interface for a key-value map. Implementations:

* [map_naive.c](chan/map_naive.c):
  * Stores the keys in a vector and performs searches in `O(n)` where `n` is the number of keys.
* [map_bst.c](chan/map_bst.c): Binary search tree. Similar to C++ `std::map`.
  * For each key, stores a pointer to the smaller key and a larger key. Performs searches in `O(log n)`.
  * Requires implementing a "less" function for the keys.
  * The iterator method produces the keys in ascending order.
* [map_hash.c](chan/map_hash.c): Hash map. Similar to C++ `std::unordered_map`.
  * Computes a hash from the key to search a previously inserted value in `O(1)`.
  * Requires implementing a "hash" function for the keys.
  * Hash collisions are handled. For example if you implement a hash that always returns `0`, the container will still work, much like `map_naive.c` (with `O(n)` search complexity).
  * The used collision resolution method is similar to what is called [open addressing on Wikipedia](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution). However the buckets do not store the values but indices of a vector where the values are stored.

Some map types do not yet implement the method to remove keys.

C++ `std::set` and `std::unordered_set` are not interesting exercises to implement since they are functionally equivalent to the corresponding map types where every value is the empty type.
