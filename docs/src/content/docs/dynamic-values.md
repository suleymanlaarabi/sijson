---
title: Dynamic Values
description: Parse, build, inspect, and stringify JSON without a reflected C type.
---

Use `sijson_value_t` when the JSON shape is not known at compile time.

```c
sijson_value_t value = sijson_parse("{\"ok\":true,\"items\":[1,2,3]}");

if (value == NULL) {
    fprintf(stderr, "%s\n", sijson_error());
    return;
}

sijson_value_t items = sijson_object_get(value, "items");
printf("items: %zu\n", sijson_array_len(items));
```

`sijson_value_t` is opaque:

```c
typedef struct sijson_value *sijson_value_t;
```

The internal node layout is intentionally hidden, so users do not depend on
allocator choices or representation details.

## Inspect values

Use `sijson_type` before reading a value.

```c
switch (sijson_type(value)) {
case SIJSON_NULL:
    puts("null");
    break;
case SIJSON_BOOL:
    printf("%s\n", sijson_bool(value) ? "true" : "false");
    break;
case SIJSON_NUMBER:
    printf("%f\n", sijson_number(value));
    break;
case SIJSON_STRING:
    puts(sijson_string(value));
    break;
case SIJSON_ARRAY:
    printf("array len: %zu\n", sijson_array_len(value));
    break;
case SIJSON_OBJECT:
    printf("object len: %zu\n", sijson_object_len(value));
    break;
}
```

## Read arrays

```c
sijson_value_t array = sijson_parse("[10,20,30]");

for (size_t i = 0; i < sijson_array_len(array); i++) {
    printf("%f\n", sijson_number(sijson_array_get(array, i)));
}
```

## Read objects

```c
sijson_value_t object = sijson_parse("{\"name\":\"Ada\",\"age\":37}");

const char *name = sijson_string(sijson_object_get(object, "name"));
double age = sijson_number(sijson_object_get(object, "age"));
```

You can also iterate object entries by index.

```c
for (size_t i = 0; i < sijson_object_len(object); i++) {
    const char *key = sijson_object_key(object, i);
    sijson_value_t member = sijson_object_get(object, key);
    printf("%s: %d\n", key, (int)sijson_type(member));
}
```

## Build JSON values

The `sijson_make_*` functions create dynamic JSON nodes.

```c
sijson_value_t user = sijson_make_object();
sijson_object_set(user, "name", sijson_make_string("Ada"));
sijson_object_set(user, "age", sijson_make_number(37));
sijson_object_set(user, "active", sijson_make_bool(true));

sijson_value_t tags = sijson_make_array();
sijson_array_push(tags, sijson_make_string("admin"));
sijson_array_push(tags, sijson_make_string("beta"));
sijson_object_set(user, "tags", tags);

char *json = sijson_value_to_str(user);
puts(json);
free(json);
```

Dynamic values live in the internal arena. Call `sijson_clean()` to invalidate
the current values and reuse the arena memory for the next batch.

## Use dynamic fields in typed structs

```c
SIJSON(Event, {
    char *name;
    sijson_value_t payload;
});

Event event = sijson_from_json(Event,
    "{\"name\":\"click\",\"payload\":{\"x\":10,\"y\":20}}");

sijson_value_t payload = event.payload;
printf("%f\n", sijson_number(sijson_object_get(payload, "x")));

sijson_free(Event, &event);
```

`sijson_free` releases heap fields owned by the struct. Dynamic JSON values are
owned by `sijson`'s internal arena.
