#include "sijson.h"
#include <sijson_test.h>

#include <math.h>
#include <stdlib.h>

void dynamic_make(void) {
    sijson_value_t root = sijson_make_object();
    test_null((void *)sijson_error());
    test_uint(sijson_type(root), SIJSON_OBJECT);

    test_assert(sijson_object_set(root, "name", sijson_make_string("Ada")));
    test_assert(sijson_object_set(root, "age", sijson_make_number(37)));
    test_assert(sijson_object_set(root, "active", sijson_make_bool(true)));
    test_assert(sijson_object_set(root, "none", sijson_make_null()));

    sijson_value_t tags = sijson_make_array();
    test_assert(sijson_array_push(tags, sijson_make_string("c")));
    test_assert(sijson_array_push(tags, sijson_make_string("json")));
    test_assert(sijson_object_set(root, "tags", tags));

    test_uint(sijson_object_len(root), 5);
    test_str(sijson_object_key(root, 0), "name");
    test_str(sijson_string(sijson_object_get(root, "name")), "Ada");
    test_flt(sijson_number(sijson_object_get(root, "age")), 37.0);
    test_true(sijson_bool(sijson_object_get(root, "active")));
    test_uint(sijson_type(sijson_object_get(root, "none")), SIJSON_NULL);
    test_uint(sijson_array_len(tags), 2);
    test_str(sijson_string(sijson_array_get(tags, 1)), "json");
}

void dynamic_arena(void) {
    sijson_value_t root = sijson_make_object();
    test_assert(sijson_object_set(root, "name", sijson_make_string("Ada")));

    const char *name = sijson_string(sijson_object_get(root, "name"));
    test_str(name, "Ada");

    sijson_clean();

    sijson_value_t next = sijson_make_object();
    test_assert(sijson_object_set(next, "name", sijson_make_string("Bob")));
    test_str(sijson_string(sijson_object_get(next, "name")), "Bob");

    char *json = sijson_value_to_str(next);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Bob\"}");
    free(json);

    sijson_release();
    sijson_value_t after_release = sijson_make_string("fresh");
    test_str(sijson_string(after_release), "fresh");
}

void dynamic_parse(void) {
    sijson_value_t root = sijson_parse(
        "{"
        "\"title\":\"hello\\nworld\","
        "\"n\":-12.5e2,"
        "\"ok\":false,"
        "\"items\":[null,true,\"x\"]"
        "}"
    );
    test_null((void *)sijson_error());
    test_uint(sijson_type(root), SIJSON_OBJECT);
    test_str(sijson_string(sijson_object_get(root, "title")), "hello\nworld");
    test_flt(sijson_number(sijson_object_get(root, "n")), -1250.0);
    test_false(sijson_bool(sijson_object_get(root, "ok")));

    sijson_value_t items = sijson_object_get(root, "items");
    test_uint(sijson_type(items), SIJSON_ARRAY);
    test_uint(sijson_array_len(items), 3);
    test_uint(sijson_type(sijson_array_get(items, 0)), SIJSON_NULL);
    test_true(sijson_bool(sijson_array_get(items, 1)));
    test_str(sijson_string(sijson_array_get(items, 2)), "x");
    test_null(sijson_array_get(items, 3));
    test_null(sijson_object_get(root, "missing"));
}

void dynamic_stringify(void) {
    sijson_value_t root = sijson_make_object();
    test_assert(sijson_object_set(root, "name", sijson_make_string("Ada")));
    test_assert(sijson_object_set(root, "age", sijson_make_number(37)));
    test_assert(sijson_object_set(root, "active", sijson_make_bool(true)));

    sijson_value_t tags = sijson_make_array();
    test_assert(sijson_array_push(tags, sijson_make_string("c")));
    test_assert(sijson_array_push(tags, sijson_make_string("json")));
    test_assert(sijson_object_set(root, "tags", tags));

    char *json = sijson_value_to_str(root);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Ada\",\"age\":37,\"active\":true,\"tags\":[\"c\",\"json\"]}");
    free(json);

    json = sijson_value_to_str(sijson_make_string("a\nb\t\"c\""));
    test_null((void *)sijson_error());
    test_str(json, "\"a\\nb\\t\\\"c\\\"\"");
    free(json);

    json = sijson_value_to_str(NULL);
    test_null((void *)sijson_error());
    test_str(json, "null");
    free(json);

    json = sijson_value_to_str(sijson_make_number(INFINITY));
    test_null(json);
    test_not_null((void *)sijson_error());
}

void dynamic_errors(void) {
    test_null(sijson_parse(NULL));
    test_not_null((void *)sijson_error());

    test_null(sijson_parse("{\"bad\":[1,}"));
    test_not_null((void *)sijson_error());

    test_null(sijson_parse("01"));
    test_not_null((void *)sijson_error());

    test_null(sijson_parse("[1.]"));
    test_not_null((void *)sijson_error());

    test_null(sijson_parse("\"bad\\q\""));
    test_not_null((void *)sijson_error());

    test_null(sijson_parse("true false"));
    test_not_null((void *)sijson_error());

    sijson_value_t object = sijson_make_object();
    test_false(sijson_array_push(object, sijson_make_null()));
    test_not_null((void *)sijson_error());

    sijson_value_t array = sijson_make_array();
    test_false(sijson_object_set(array, "x", sijson_make_null()));
    test_not_null((void *)sijson_error());
}

void dynamic_registry(void) {
    test_not_null(sijson_default_registry());
    test_null((void *)sijson_error());
}
