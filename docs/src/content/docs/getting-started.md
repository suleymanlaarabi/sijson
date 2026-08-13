---
title: Getting Started
description: Declare a JSON-reflectable C struct and convert it to and from JSON.
---

This page shows the normal `sijson` flow:

1. Declare a struct with `SIJSON`.
2. Serialize a value with `sijson_to_json`.
3. Deserialize JSON with `sijson_from_json`.
4. Release deserialized heap fields with `sijson_free`.

Typed operations require an active Sireflect context. Call `sireflect_init()`
before the first typed operation and `sireflect_fini()` after the last one.

## Include sijson

```c
#include <sijson.h>
```

`sijson.h` includes `sireflect.h`, `stdbool.h`, and `stddef.h`.

Before using a reflected type, initialize Sireflect. Keep the context alive
for all typed operations, then release it once those operations are complete.

```c
sireflect_init();
/* typed sijson operations */
sireflect_fini();
```

## Declare a type

For a type used in one translation unit, use `SIJSON`.

```c
SIJSON(User, {
    char *name;
    int age;
    bool active;
});
```

This declares the C type, captures its field list for `sireflect`, and creates
the internal type handle used by `sijson`.

## Serialize a compound literal

```c
char *json = sijson_to_json(User, {
    .name = "Ada",
    .age = 37,
    .active = true,
});

puts(json);
free(json);
```

`sijson_to_json` returns a newly allocated string. The caller owns it and must
release it with `free`.

## Serialize an existing object

Use `sijson_to_json_ptr` when you already have a variable.

```c
User user = {
    .name = "Ada",
    .age = 37,
    .active = true,
};

char *json = sijson_to_json_ptr(User, &user);
free(json);
```

## Deserialize a value

```c
User user = sijson_from_json(User, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");

printf("%s\n", user.name);

sijson_free(User, &user);
```

The returned struct value is copied out of an internal temporary buffer. Heap
fields inside the struct, such as `char *`, belong to the caller and must be
released with `sijson_free(Type, &value)`.

## Handle errors

Most public operations return `NULL`, `false`, or an invalid value on failure.
Read the reason with `sijson_error()`.

```c
User user = sijson_from_json(User, "{\"name\":42}");
const char *error = sijson_error();

if (error != NULL) {
    fprintf(stderr, "json error: %s\n", error);
}
```

## Split declarations across files

Use `SIJSON_DECLARE` in a header and `SIJSON_DEFINE` in exactly one `.c` file.

```c
/* user.h */
#include <sijson.h>

SIJSON_DECLARE(User, {
    char *name;
    int age;
    bool active;
});
```

```c
/* user.c */
#include "user.h"

SIJSON_DEFINE(User)
```
