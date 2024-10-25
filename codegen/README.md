# Serializing Structured Data

## Problem Description

We have a structure `obj` which we want to (de)serialize (also known as _(un)marshal_) from/into a buffer `buf` in order to communicate over a socket.

Users of the serialization framework will have two use cases:

- **marshalling**: `obj` -> `buf`
- **unmarshalling**: `buf` -> `obj`

In the latter case, memory allocation by the serialization framework might be required (especially when length is unknown beforehand).
A `free` function must therefore be provided.

## Solution

(Un)marshal functions can be generated programmatically when the structured data is specified.

**Example**:

```c
struct x {
    uint8_t len; // length of array
    uint32_t *arr;
    struct c opt; // only contains valid data if len == 10
};
```

The complex structure type `x` has three members of types `uint8_t`, `uint32_t` and `c`, where `c` is another complex structure type.

We can specify `x` in a tree structure:

```text
             x
     / ------|----- \     modifier: [name, optional?, array (variable, fixed)]
    /        |       \
uint8_t   uint32_t    c
```

_Primitives_ such as `uint8_t` or `uint64_t` can be directly serialized into a series of bytes.
Complex types such as `x` or `c` have to be serialized depth-first.

By using modifiers, the members of `x` can further be specified.
Notice that during serialization, `obj` is known whereas during deserialization, the modifiers can only be known when they are transmitted before the actual members (e.g. send the length of an array before the array).
Optional values keep the occupied space of `buf` as low as possible.
