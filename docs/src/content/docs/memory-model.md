---
title: Memory Model
description: Ownership rules for JSON strings, deserialized structs, and dynamic values.
---

The memory rules are intentionally small, but they matter.

## Serialized JSON strings

`sijson_to_json`, `sijson_to_json_ptr`, and `sijson_stringify` return newly
allocated strings.

```c
char *json = sijson_to_json(User, { .name = "Ada", .age = 37 });
free(json);
```

The caller owns these strings and releases them with `free`.

## Deserialized structs

`sijson_from_json(Type, json)` returns a C struct value.

```c
User user = sijson_from_json(User, json);
```

Internally, `sijson` writes the struct into a reusable temporary buffer, grows
that buffer when necessary, and the macro copies the value back to the caller.
A later `sijson_from_json` call can reuse the internal buffer.

The struct value itself is yours. Heap fields inside it are yours too.

```c
printf("%s\n", user.name);
sijson_free(User, &user);
```

Call `sijson_free(Type, &value)` once for each successfully deserialized struct
that contains heap-owned fields such as `char *`.

## String fields

`char *` fields deserialized from JSON strings are heap allocations owned by
the caller after `sijson_from_json`.

```c
User user = sijson_from_json(User, "{\"name\":\"Ada\"}");

/* user.name is valid until sijson_free(User, &user). */
puts(user.name);

sijson_free(User, &user);
```

`sijson_free` does not free the struct pointer itself. It only releases fields
inside the struct according to reflected metadata.

## Dynamic values

Values returned by `sijson_parse` and `sijson_make_*` are owned by `sijson`'s
internal context.

```c
sijson_value_t value = sijson_parse("{\"debug\":true}");
```

Do not call `free` on a `sijson_value_t`. The handle is opaque and its storage
belongs to the library.

When a reflected struct contains a `sijson_value_t` field, the field points to a
dynamic JSON value owned by that same internal context.

## Practical rules

- Free every JSON string returned by serialization/stringification.
- Free every successful deserialized struct with `sijson_free(Type, &value)` if
  it may contain owned fields.
- Do not free `sijson_value_t`.
- Do not keep pointers returned by dynamic accessors longer than the internal
  context lifetime.
- Treat the current API as not thread-safe unless you add external
  synchronization.
