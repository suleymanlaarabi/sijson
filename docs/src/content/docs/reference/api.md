---
title: API Reference
description: Public symbols from sijson.h.
---

Include the public API with:

```c
#include <sijson.h>
```

## Type declaration macros

### `SIJSON(type, fields)`

Declares and defines a JSON-reflectable type in the current translation unit.

```c
SIJSON(User, {
    char *name;
    int age;
});
```

### `SIJSON_DECLARE(type, fields)`

Declares a JSON-reflectable type in a header.

```c
SIJSON_DECLARE(User, {
    char *name;
    int age;
});
```

### `SIJSON_DEFINE(type)`

Defines storage for a type declared with `SIJSON_DECLARE`. Use it in exactly one
source file.

```c
SIJSON_DEFINE(User)
```

### `sijson_handle(type)`

Expands to the internal reflected type handle symbol for `type`.
Most code does not need this directly, except when calling low-level functions.

## Registry

```c
sireflect_registry_t *sijson_default_registry(void);
```

Returns the default `sireflect` registry used by the convenience API. The
registry is initialized lazily.

## Typed serialization

```c
#define sijson_to_json(type, ...)
#define sijson_to_json_ptr(type, ptr)

char *sijson_to_json_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    const void *ptr
);
```

`sijson_to_json` serializes a compound initializer.

```c
char *json = sijson_to_json(User, { .name = "Ada", .age = 37 });
free(json);
```

`sijson_to_json_ptr` serializes an existing object.

```c
User user = { .name = "Ada", .age = 37 };
char *json = sijson_to_json_ptr(User, &user);
free(json);
```

Both return a newly allocated string owned by the caller, or `NULL` on failure.

## Typed deserialization

```c
#define sijson_from_json(type, json)

void *sijson_from_json_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    const char *json
);
```

`sijson_from_json` deserializes JSON into a struct value.

```c
User user = sijson_from_json(User, json);
sijson_free(User, &user);
```

The low-level function returns a pointer to the internal temporary buffer, or
`NULL` on failure.

## Free deserialized fields

```c
#define sijson_free(type, ptr)

void sijson_free_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    void *ptr
);
```

Releases heap-owned fields inside a struct produced by `sijson_from_json`.
It does not free the struct object itself.

```c
User user = sijson_from_json(User, json);
sijson_free(User, &user);
```

## Dynamic value types

```c
typedef struct sijson_value *sijson_value_t;

typedef enum sijson_type {
    SIJSON_NULL,
    SIJSON_BOOL,
    SIJSON_NUMBER,
    SIJSON_STRING,
    SIJSON_ARRAY,
    SIJSON_OBJECT,
} sijson_type_t;
```

`sijson_value_t` is an opaque handle for arbitrary JSON.

## Parse and stringify dynamic JSON

```c
sijson_value_t sijson_parse(const char *json);
char *sijson_stringify(sijson_value_t value);
```

`sijson_parse` returns a dynamic JSON value owned by `sijson`, or `NULL` on
failure.

`sijson_stringify` returns a newly allocated JSON string owned by the caller, or
`NULL` on failure.

## Inspect dynamic values

```c
sijson_type_t sijson_type(sijson_value_t value);

bool sijson_bool(sijson_value_t value);
double sijson_number(sijson_value_t value);
const char *sijson_string(sijson_value_t value);
```

Call the scalar reader that matches `sijson_type(value)`.

## Arrays

```c
size_t sijson_array_len(sijson_value_t value);
sijson_value_t sijson_array_get(sijson_value_t value, size_t index);
bool sijson_array_push(sijson_value_t array, sijson_value_t value);
```

`sijson_array_get` returns `NULL` if the value is not an array or the index is
out of range.

`sijson_array_push` appends a dynamic value to an array and returns `false` on
failure.

## Objects

```c
size_t sijson_object_len(sijson_value_t value);
const char *sijson_object_key(sijson_value_t value, size_t index);
sijson_value_t sijson_object_get(sijson_value_t value, const char *key);
bool sijson_object_set(sijson_value_t object, const char *key, sijson_value_t value);
```

`sijson_object_get` returns `NULL` when the member does not exist or when
`object` is not a JSON object.

`sijson_object_set` inserts or replaces a member and returns `false` on failure.

## Build dynamic JSON

```c
sijson_value_t sijson_make_null(void);
sijson_value_t sijson_make_bool(bool value);
sijson_value_t sijson_make_number(double value);
sijson_value_t sijson_make_string(const char *value);
sijson_value_t sijson_make_array(void);
sijson_value_t sijson_make_object(void);
```

All dynamic values are owned by `sijson`'s internal context.

## Errors

```c
const char *sijson_error(void);
```

Returns the last error message, or `NULL` when no error is currently stored.
