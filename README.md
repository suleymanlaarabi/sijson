![SIJSON](docs/assets/banner.png)

[![Documentation](https://img.shields.io/badge/docs-sijson-blue?style=for-the-badge&color=blue)](https://suleymanlaarabi.github.io/sijson/)
[![actions](https://img.shields.io/github/actions/workflow/status/suleymanlaarabi/sijson/docs.yml?branch=main&style=for-the-badge)](https://github.com/suleymanlaarabi/sijson/actions?query=workflow%3ACI)

`sijson` is a small JSON serialization and deserialization library for modern C23.

It builds on top of [`sireflect`](https://github.com/suleymanlaarabi/sireflect) so you can describe a C struct once, then convert it to and from JSON with a compact API.

- Typed JSON serialization for reflected C structs.
- Typed JSON deserialization with automatic cleanup helpers for heap-owned fields.
- Dynamic JSON parsing, inspection, construction, and stringification through `sijson_value_t`.
- Mixed typed/dynamic payloads by embedding `sijson_value_t` fields in reflected structs.
- Small error surface through `sijson_error()`.
- C23 API designed to stay simple at the call site.

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

    if (json == NULL) {
        fprintf(stderr, "%s\n", sijson_error());
        return 1;
    }

    puts(json);
    free(json);

    User user = sijson_from_json(User, "{\"name\":\"Bob\",\"age\":41,\"active\":false}");

    if (sijson_error() != NULL) {
        fprintf(stderr, "%s\n", sijson_error());
        return 1;
    }

    printf("%s is %d\n", user.name, user.age);
    sijson_free(User, &user);

    return 0;
}
```

## Dynamic JSON values

Use `sijson_value_t` when the JSON shape is not known at compile time.

```c
sijson_value_t root = sijson_parse("{\"ok\":true,\"items\":[1,2,3]}");

if (root == NULL) {
    fprintf(stderr, "%s\n", sijson_error());
    return 1;
}

sijson_value_t items = sijson_object_get(root, "items");

for (size_t i = 0; i < sijson_array_len(items); i++) {
    printf("%f\n", sijson_number(sijson_array_get(items, i)));
}

sijson_clean();
```

Dynamic values are stored in `sijson`'s internal arena. Call `sijson_clean()` to reuse the arena, or `sijson_release()` to free it.

## Installation

The repository provides amalgamated distribution files in `distr/`.

## Documentation

- [Documentation](https://suleymanlaarabi.github.io/sijson/)
- [Getting Started](https://suleymanlaarabi.github.io/sijson/getting-started/)
- [Dynamic Values](https://suleymanlaarabi.github.io/sijson/dynamic-values/)
- [Memory Model](https://suleymanlaarabi.github.io/sijson/memory-model/)
- [API Reference](https://suleymanlaarabi.github.io/sijson/reference/api/)
