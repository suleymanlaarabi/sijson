#include "sijson.h"
#include <sijson_test.h>

#include <math.h>
#include <stdlib.h>

SIJSON(User, {
    char *name;
    int age;
    bool active;
});

SIJSON(Profile, {
    char *name;
    char initial;
    u8 level;
    i16 score;
    u32 visits;
    f32 ratio;
    f64 weight;
    bool enabled;
});

SIJSON(Event, {
    char *name;
    sijson_value_t payload;
});

void simple_dynamic_make(void) {
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

void simple_dynamic_parse(void) {
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

void simple_stringify(void) {
    sijson_value_t root = sijson_make_object();
    test_assert(sijson_object_set(root, "name", sijson_make_string("Ada")));
    test_assert(sijson_object_set(root, "age", sijson_make_number(37)));
    test_assert(sijson_object_set(root, "active", sijson_make_bool(true)));

    sijson_value_t tags = sijson_make_array();
    test_assert(sijson_array_push(tags, sijson_make_string("c")));
    test_assert(sijson_array_push(tags, sijson_make_string("json")));
    test_assert(sijson_object_set(root, "tags", tags));

    char *json = sijson_stringify(root);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Ada\",\"age\":37,\"active\":true,\"tags\":[\"c\",\"json\"]}");
    free(json);

    json = sijson_stringify(sijson_make_string("a\nb\t\"c\""));
    test_null((void *)sijson_error());
    test_str(json, "\"a\\nb\\t\\\"c\\\"\"");
    free(json);

    json = sijson_stringify(NULL);
    test_null((void *)sijson_error());
    test_str(json, "null");
    free(json);

    json = sijson_stringify(sijson_make_number(INFINITY));
    test_null(json);
    test_not_null((void *)sijson_error());
}

void simple_dynamic_errors(void) {
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

void simple_registry(void) {
    test_not_null(sijson_default_registry());
    test_null((void *)sijson_error());
}

void simple_typed_to_json(void) {
    User user = {
        .name = "Ada",
        .age = 37,
        .active = true,
    };

    char *json = sijson_to_json_ptr(User, &user);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");
    free(json);

    json = sijson_to_json(User, { .name = "Bob", .age = 41, .active = false });
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Bob\",\"age\":41,\"active\":false}");
    free(json);

    user.name = NULL;
    json = sijson_to_json_ptr(User, &user);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":null,\"age\":37,\"active\":true}");
    free(json);
}

void simple_typed_from_json(void) {
    User user = sijson_from_json(User, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");
    test_null((void *)sijson_error());
    test_str(user.name, "Ada");
    test_int(user.age, 37);
    test_true(user.active);
    sijson_free(User, &user);
    test_null(user.name);

    user = sijson_from_json(User, "{\"name\":null,\"age\":0,\"active\":false}");
    test_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);
    test_false(user.active);

    Profile profile = sijson_from_json(
        Profile,
        "{"
        "\"name\":\"Ada\","
        "\"initial\":\"A\","
        "\"level\":7,"
        "\"score\":-12,"
        "\"visits\":42,"
        "\"ratio\":1.5,"
        "\"weight\":2.25,"
        "\"enabled\":true"
        "}"
    );
    test_null((void *)sijson_error());
    test_str(profile.name, "Ada");
    test_int(profile.initial, 'A');
    test_uint(profile.level, 7);
    test_int(profile.score, -12);
    test_uint(profile.visits, 42);
    test_flt(profile.ratio, 1.5);
    test_flt(profile.weight, 2.25);
    test_true(profile.enabled);
    sijson_free(Profile, &profile);
    test_null(profile.name);
}

void simple_typed_errors(void) {
    User user = sijson_from_json(User, "{\"name\":\"Ada\",\"ignored\":123}");
    test_null((void *)sijson_error());
    test_str(user.name, "Ada");
    test_int(user.age, 0);
    test_false(user.active);
    sijson_free(User, &user);

    user = sijson_from_json(User, "{\"name\":42}");
    test_not_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);

    user = sijson_from_json(User, "{\"age\":1.5}");
    test_not_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);

    char *json = sijson_to_json_ptr(User, NULL);
    test_null(json);
    test_not_null((void *)sijson_error());
}

void simple_typed_dynamic_value(void) {
    Event event =
        sijson_from_json(Event, "{\"name\":\"created\",\"payload\":{\"kind\":\"user\",\"id\":12}}");
    test_null((void *)sijson_error());
    test_str(event.name, "created");
    test_uint(sijson_type(event.payload), SIJSON_OBJECT);
    test_str(sijson_string(sijson_object_get(event.payload, "kind")), "user");
    test_flt(sijson_number(sijson_object_get(event.payload, "id")), 12.0);

    char *json = sijson_to_json_ptr(Event, &event);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"created\",\"payload\":{\"kind\":\"user\",\"id\":12}}");
    free(json);

    sijson_free(Event, &event);
    test_null(event.name);
}
