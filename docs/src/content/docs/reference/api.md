---
title: API Reference
description: sijson API Reference.
---

`sijson` is designed to be used through its C23-friendly macros.

## Typed JSON

Typed operations require an active Sireflect context. Pair every
`sireflect_init()` with one `sireflect_fini()` after the last typed operation.
Do not use reflected handles or metadata after the final `fini`.

```c
sireflect_init();
/* typed JSON operations */
sireflect_fini();
```

```c
#define sijson_to_json(type, ...)
#define sijson_to_json_ptr(type, ptr)
#define sijson_from_json(type, json)
#define sijson_free(type, ptr)
```

`sijson_to_json` serializes a value written as a compound initializer. It
returns a newly allocated JSON string owned by the caller, or `NULL` on failure.

`sijson_to_json_ptr` serializes a value from a pointer. It returns a newly
allocated JSON string owned by the caller, or `NULL` on failure.

`sijson_from_json` deserializes a JSON string into a struct. The returned
struct is copied from an internal temporary buffer and stays valid until the
end of the expression.

`sijson_free` releases heap-owned fields (like `char *`) inside a struct
produced by `sijson_from_json`. It does not free the struct pointer itself.

## Dynamic JSON

```c
sijson_value_t sijson_parse(const char *json);
char *sijson_stringify(sijson_value_t value);
```

`sijson_parse` returns a dynamic JSON value allocated in `sijson`'s internal
arena, or `NULL` on failure.

`sijson_stringify` returns a newly allocated JSON string owned by the caller,
or `NULL` on failure.

## Dynamic arena lifetime

```c
void sijson_clean(void);
void sijson_release(void);
```

`sijson_clean` invalidates every existing `sijson_value_t` and marks the
internal arena memory as reusable.

`sijson_release` invalidates every existing `sijson_value_t` and frees the
internal arena storage.

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

All dynamic values are allocated in `sijson`'s internal arena.

## Errors

```c
const char *sijson_error(void);
```

Returns the last error message, or `NULL` when no error is currently stored.
