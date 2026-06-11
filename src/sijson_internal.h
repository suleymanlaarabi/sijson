#ifndef SIJSON_INTERNAL_H
#define SIJSON_INTERNAL_H

#include "sijson.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define SIJSON_INTERNAL_API __attribute__((visibility("hidden")))
#else
#define SIJSON_INTERNAL_API
#endif

typedef struct sijson_member {
    char *key;
    sijson_value_t value;
} sijson_member_t;

typedef struct sijson_array {
    sijson_value_t *items;
    size_t len;
    size_t cap;
} sijson_array_t;

typedef struct sijson_object {
    sijson_member_t *items;
    size_t len;
    size_t cap;
} sijson_object_t;

struct sijson_value {
    sijson_type_t type;
    union {
        bool boolean;
        double number;
        char *string;
        sijson_array_t array;
        sijson_object_t object;
    } as;
};

typedef struct sijson_writer {
    char *data;
    size_t len;
    size_t cap;
} sijson_writer_t;

typedef struct sijson_parser {
    const char *cur;
} sijson_parser_t;

SIJSON_INTERNAL_API void sijson_clear_error(void);
SIJSON_INTERNAL_API bool sijson_set_error(const char *message);
SIJSON_INTERNAL_API bool sijson_set_error_at(const char *message, const char *at);

SIJSON_INTERNAL_API char *sijson_dup_range(const char *start, size_t len);
SIJSON_INTERNAL_API char *sijson_dup_cstr(const char *str);

SIJSON_INTERNAL_API void *sijson_arena_alloc(size_t size, size_t align);
SIJSON_INTERNAL_API char *sijson_arena_dup_range(const char *start, size_t len);
SIJSON_INTERNAL_API char *sijson_arena_dup_cstr(const char *str);
SIJSON_INTERNAL_API size_t sijson_arena_mark(void);
SIJSON_INTERNAL_API void sijson_arena_rewind(size_t mark);

SIJSON_INTERNAL_API bool sijson_reserve_array(sijson_array_t *array, size_t need);
SIJSON_INTERNAL_API bool sijson_reserve_object(sijson_object_t *object, size_t need);
SIJSON_INTERNAL_API sijson_value_t sijson_new_value(sijson_type_t type);

SIJSON_INTERNAL_API bool sijson_writer_reserve(sijson_writer_t *writer, size_t extra);
SIJSON_INTERNAL_API bool sijson_writer_putc(sijson_writer_t *writer, char c);
SIJSON_INTERNAL_API bool sijson_writer_write(sijson_writer_t *writer, const char *data, size_t len);
SIJSON_INTERNAL_API bool sijson_writer_cstr(sijson_writer_t *writer, const char *str);
SIJSON_INTERNAL_API bool sijson_writer_string(sijson_writer_t *writer, const char *str);
SIJSON_INTERNAL_API bool sijson_write_value(sijson_writer_t *writer, sijson_value_t value);

#endif
