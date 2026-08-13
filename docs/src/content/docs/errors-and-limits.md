---
title: Errors and Limits
description: Failure handling and current implementation boundaries.
---

Use `sijson_error()` after a failed operation.

```c
sijson_value_t value = sijson_parse("{bad json");

if (value == NULL) {
    fprintf(stderr, "parse failed: %s\n", sijson_error());
}
```

## Failure results

| Operation | Failure result |
| --- | --- |
| `sijson_parse` | `NULL` |
| `sijson_stringify` | `NULL` |
| `sijson_stringify` | `NULL` |
| `sijson_to_json` | `NULL` |
| `sijson_to_json_ptr` | `NULL` |
| `sijson_from_json_impl` | `NULL` |
| `sijson_array_push` | `false` |
| `sijson_object_set` | `false` |

`sijson_from_json(Type, json)` is a convenience macro for successful code paths.
If you need to handle failure before dereferencing, use the low-level
`sijson_from_json_impl` function.

```c
void *ptr = sijson_from_json_impl(
    &sijson_handle(User),
    &sireflect_desc(User),
    json
);

if (ptr == NULL) {
    fprintf(stderr, "%s\n", sijson_error());
    return;
}

User user = *(User *)ptr;
```

## Type errors

Typed deserialization expects JSON values to match reflected field types.

```json
{ "name": 42 }
```

This is invalid for:

```c
SIJSON(User, {
    char *name;
});
```

## Integer numbers

Integer fields require JSON numbers without a fractional part.

```json
{ "age": 37 }
```

is valid, but:

```json
{ "age": 37.5 }
```

is not valid for an `int`, `u32`, or other integer field.

## Current limits

These limits come from the current `sijson` and `sireflect` implementation:

| Limit | Notes |
| --- | --- |
| C arrays | Not represented by current reflection metadata. |
| `const char *` | Pointer field metadata is erased to `ptr`; use `char *`. |
| Pointer types other than `char *` | Reflected as raw `ptr`, not enough for typed JSON. |
| Packed structs and bitfields | Outside the supported `sireflect` layout subset. |
| Non-finite numbers | JSON has no `NaN` or infinity values. |
| Thread safety | The global Sireflect context and internal arena are not synchronized. |

## Unknown JSON shape

When a JSON field is intentionally open-ended, use `sijson_value_t` instead of
trying to model every possible object shape as C fields.

```c
SIJSON(Message, {
    char *type;
    sijson_value_t body;
});
```
