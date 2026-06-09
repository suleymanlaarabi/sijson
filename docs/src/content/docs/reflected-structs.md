---
title: Reflected Structs
description: How sijson uses sireflect declarations for typed JSON.
---

`sijson` gets struct metadata from `sireflect`. A JSON-capable type must be
declared through one of the `SIJSON` macros.

## One-file declaration

```c
SIJSON(Position, {
    f32 x;
    f32 y;
});
```

Use this form for examples, tests, and private types.

## Header/source declaration

```c
/* position.h */
SIJSON_DECLARE(Position, {
    f32 x;
    f32 y;
});
```

```c
/* position.c */
SIJSON_DEFINE(Position)
```

Use this form when the type is shared across translation units.

## Supported field types

`sijson` supports the primitive names exposed by `sireflect`, plus strings and
dynamic JSON values.

| C field | JSON representation |
| --- | --- |
| `bool` | `true` or `false` |
| signed integers | JSON number, without fractional part |
| unsigned integers | JSON number, without fractional part |
| `float`, `double`, `f32`, `f64` | JSON number |
| `char` | one-character JSON string |
| `char *` | JSON string or `null` |
| nested `SIJSON` struct | JSON object |
| `sijson_value_t` | arbitrary JSON value |

```c
SIJSON(Profile, {
    char *name;
    u32 visits;
    f64 weight;
    bool enabled;
});
```

## Nested structs

Register the nested type before it is used through serialization or
deserialization.

```c
SIJSON(Position, {
    f32 x;
    f32 y;
});

SIJSON(Entity, {
    char *name;
    Position position;
});
```

## Arbitrary JSON fields

Use `sijson_value_t` when a field can contain any JSON shape.

```c
SIJSON(Event, {
    char *name;
    sijson_value_t payload;
});
```

This is useful for metadata, extension objects, protocol messages, and user
defined settings.

## Current syntax limits

The struct declaration is still parsed by `sireflect`, so the same declaration
subset applies. Avoid arrays, bitfields, qualifiers such as `const`, packed
layout attributes, and multi-declarator fields such as `int x, y;`.

Use one field declaration per line:

```c
SIJSON(Point, {
    int x;
    int y;
});
```
