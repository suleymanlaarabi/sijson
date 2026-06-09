---
title: sijson
description: Stupidly simple JSON serialization and deserialization for reflected C23 structs.
---

`sijson` is a small JSON serialization layer built on top of
[`sireflect`](https://suleymanlaarabi.github.io/sireflect/).

It is designed around one idea: describe a C struct once, then serialize and
deserialize it with a tiny API.

```c
#include <sijson.h>
#include <stdio.h>
#include <stdlib.h>

SIJSON(User, {
    char *name;
    int age;
    bool active;
});

int main(void) {
    char *json = sijson_to_json(User, {
        .name = "Ada",
        .age = 37,
        .active = true,
    });

    printf("%s\n", json);
    free(json);

    User user = sijson_from_json(User, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");
    printf("%s\n", user.name);

    sijson_free(User, &user);
    return 0;
}
```

## What sijson provides

| Feature | Description |
| --- | --- |
| Reflected structs | JSON support for structs declared with `SIJSON`, `SIJSON_DECLARE`, or `SIJSON_DEFINE`. |
| Typed serialization | Convert a C struct into a newly allocated JSON string. |
| Typed deserialization | Convert JSON into a C struct value with a simple `Type value = ...` expression. |
| Dynamic JSON values | Parse, inspect, build, and stringify JSON when the shape is not known at compile time. |
| Mixed typed/dynamic fields | Use `sijson_value_t` inside a reflected struct for arbitrary JSON payloads. |
| Small error API | Read the last failure with `sijson_error()`. |

## Design goals

- Keep the public API obvious at the call site.
- Make the common path short: `sijson_to_json(Type, {...})` and
  `sijson_from_json(Type, json)`.
- Let `sireflect` own type metadata and keep `sijson` focused on JSON.
- Avoid exposing internal JSON node layout to users.
- Keep generated strings caller-owned and make struct-owned heap fields
  releasable with one matching `sijson_free(Type, &value)` call.

## Start here

1. Read [Getting Started](./getting-started/) for the smallest full example.
2. Read [Reflected Structs](./reflected-structs/) for type declarations and supported fields.
3. Read [Dynamic Values](./dynamic-values/) for unknown JSON shapes and `sijson_value_t`.
4. Read [Memory Model](./memory-model/) before keeping deserialized values around.
5. Use the [API Reference](./reference/api/) as the compact symbol list.
