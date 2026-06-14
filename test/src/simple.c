#include "sijson.h"
#include <sijson_test.h>

#include <limits.h>
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

SIJSON(Vector, {
    float values[4];
});

SIJSON(IntList, {
    int ids[3];
});

SIJSON(Matrix, {
    float values[2][2];
});

SIJSON(ArrayStructItem, {
    int id;
    bool active;
    char *label;
});

SIJSON(ArrayStructBox, {
    ArrayStructItem items[2];
});

SIJSON(ArrayStrings, {
    char *names[2];
});

SIJSON(NativeAliases, {
    signed char code;
    unsigned char byte;
    unsigned short small;
    unsigned int flags;
    unsigned long span;
    long long count;
    unsigned long long mask;
});

SIJSON(NativeAliasArrays, {
    unsigned long long values[2];
    signed char codes[3];
});

static void simple_register_array_struct_item(void) {
    ArrayStructItem item = { 0 };
    char *json = sijson_to_json_ptr(ArrayStructItem, &item);
    test_null((void *)sijson_error());
    free(json);
}

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

void simple_dynamic_arena(void) {
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

void simple_typed_array_to_json_numbers(void) {
    Vector vector = { .values = { 1.25f, 2.5f, 3.75f, 4.0f } };

    char *json = sijson_to_json_ptr(Vector, &vector);
    test_null((void *)sijson_error());
    test_str(json, "{\"values\":[1.25,2.5,3.75,4]}");
    free(json);

    IntList list = { .ids = { 7, 8, 9 } };
    json = sijson_to_json_ptr(IntList, &list);
    test_null((void *)sijson_error());
    test_str(json, "{\"ids\":[7,8,9]}");
    free(json);
}

void simple_typed_array_from_json_numbers(void) {
    Vector vector = sijson_from_json(Vector, "{\"values\":[1.25,2.5,3.75,4]}");
    test_null((void *)sijson_error());
    test_flt(vector.values[0], 1.25);
    test_flt(vector.values[1], 2.5);
    test_flt(vector.values[2], 3.75);
    test_flt(vector.values[3], 4.0);

    IntList list = sijson_from_json(IntList, "{\"ids\":[7,8,9]}");
    test_null((void *)sijson_error());
    test_int(list.ids[0], 7);
    test_int(list.ids[1], 8);
    test_int(list.ids[2], 9);
}

void simple_typed_array_roundtrip_matrix(void) {
    Matrix matrix = { .values = { { 1.0f, 2.0f }, { 3.5f, 4.25f } } };

    char *json = sijson_to_json_ptr(Matrix, &matrix);
    test_null((void *)sijson_error());
    test_str(json, "{\"values\":[[1,2],[3.5,4.25]]}");

    Matrix parsed = sijson_from_json(Matrix, json);
    test_null((void *)sijson_error());
    test_flt(parsed.values[0][0], 1.0);
    test_flt(parsed.values[0][1], 2.0);
    test_flt(parsed.values[1][0], 3.5);
    test_flt(parsed.values[1][1], 4.25);
    free(json);
}

void simple_typed_array_of_structs(void) {
    simple_register_array_struct_item();

    ArrayStructBox box = {
        .items = {
            { .id = 1, .active = true, .label = "one" },
            { .id = 2, .active = false, .label = "two" },
        },
    };

    char *json = sijson_to_json_ptr(ArrayStructBox, &box);
    test_null((void *)sijson_error());
    test_str(
        json,
        "{\"items\":[{\"id\":1,\"active\":true,\"label\":\"one\"},{\"id\":2,\"active\":false,"
        "\"label\":\"two\"}]}"
    );
    free(json);

    box = sijson_from_json(
        ArrayStructBox,
        "{\"items\":[{\"id\":3,\"active\":false,\"label\":\"three\"},{\"id\":4,\"active\":true,"
        "\"label\":\"four\"}]}"
    );
    test_null((void *)sijson_error());
    test_int(box.items[0].id, 3);
    test_false(box.items[0].active);
    test_str(box.items[0].label, "three");
    test_int(box.items[1].id, 4);
    test_true(box.items[1].active);
    test_str(box.items[1].label, "four");

    sijson_free(ArrayStructBox, &box);
    test_null(box.items[0].label);
    test_null(box.items[1].label);
}

void simple_typed_array_of_strings(void) {
    ArrayStrings strings = sijson_from_json(ArrayStrings, "{\"names\":[\"Ada\",\"Bob\"]}");
    test_null((void *)sijson_error());
    test_str(strings.names[0], "Ada");
    test_str(strings.names[1], "Bob");

    char *json = sijson_to_json_ptr(ArrayStrings, &strings);
    test_null((void *)sijson_error());
    test_str(json, "{\"names\":[\"Ada\",\"Bob\"]}");
    free(json);

    sijson_free(ArrayStrings, &strings);
    test_null(strings.names[0]);
    test_null(strings.names[1]);
}

void simple_typed_array_rejects_wrong_length(void) {
    Vector vector = sijson_from_json(Vector, "{\"values\":[1,2]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);
    test_flt(vector.values[3], 0.0);

    vector = sijson_from_json(Vector, "{\"values\":[1,2,3,4,5]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);
}

void simple_typed_array_rejects_wrong_type(void) {
    Vector vector = sijson_from_json(Vector, "{\"values\":[1,\"bad\",3,4]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);

    Matrix matrix = sijson_from_json(Matrix, "{\"values\":[[1,2],[3]]}");
    test_not_null((void *)sijson_error());
    test_flt(matrix.values[0][0], 0.0);
}

void simple_typed_native_aliases_to_json(void) {
    NativeAliases aliases = {
        .code = -12,
        .byte = 250,
        .small = 65000,
        .flags = 4000000000u,
        .span = 123456789ul,
        .count = -9000000000ll,
        .mask = 1800000000000000ull,
    };

    char *json = sijson_to_json_ptr(NativeAliases, &aliases);
    test_null((void *)sijson_error());
    test_str(
        json,
        "{\"code\":-12,\"byte\":250,\"small\":65000,\"flags\":4000000000,\"span\":123456789,"
        "\"count\":-9000000000,\"mask\":1800000000000000}"
    );
    free(json);

    NativeAliasArrays arrays = {
        .values = { 11ull, 22ull },
        .codes = { -1, 0, 1 },
    };
    json = sijson_to_json_ptr(NativeAliasArrays, &arrays);
    test_null((void *)sijson_error());
    test_str(json, "{\"values\":[11,22],\"codes\":[-1,0,1]}");
    free(json);
}

void simple_typed_native_aliases_from_json(void) {
    NativeAliases aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":-12,\"byte\":250,\"small\":65000,\"flags\":4000000000,\"span\":123456789,"
        "\"count\":-9000000000,\"mask\":1800000000000000}"
    );
    test_null((void *)sijson_error());
    test_int(aliases.code, -12);
    test_uint(aliases.byte, 250);
    test_uint(aliases.small, 65000);
    test_uint(aliases.flags, 4000000000u);
    test_uint(aliases.span, 123456789ul);
    test_int(aliases.count, -9000000000ll);
    test_uint(aliases.mask, 1800000000000000ull);

    NativeAliasArrays arrays =
        sijson_from_json(NativeAliasArrays, "{\"values\":[11,22],\"codes\":[-1,0,1]}");
    test_null((void *)sijson_error());
    test_uint(arrays.values[0], 11);
    test_uint(arrays.values[1], 22);
    test_int(arrays.codes[0], -1);
    test_int(arrays.codes[1], 0);
    test_int(arrays.codes[2], 1);
}

void simple_typed_native_aliases_reject_range(void) {
    NativeAliases aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":128,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_int(aliases.code, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":-1,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.byte, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":65536,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.small, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":1.5,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_int(aliases.count, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":-1}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.mask, 0);
}
