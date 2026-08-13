#include "sijson.h"
#include <sijson_test.h>

#include <stdlib.h>

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

static void typed_array_register_struct_item(void) {
    ArrayStructItem item = { 0 };
    char *json = sijson_to_json_ptr(ArrayStructItem, &item);
    test_null((void *)sijson_error());
    free(json);
}

void typed_array_to_json_numbers(void) {
    sireflect_init();
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
    sireflect_fini();
}

void typed_array_from_json_numbers(void) {
    sireflect_init();
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
    sireflect_fini();
}

void typed_array_roundtrip_matrix(void) {
    sireflect_init();
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
    sireflect_fini();
}

void typed_array_of_structs(void) {
    sireflect_init();
    typed_array_register_struct_item();

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
    sireflect_fini();
}

void typed_array_of_strings(void) {
    sireflect_init();
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
    sireflect_fini();
}

void typed_array_rejects_wrong_length(void) {
    sireflect_init();
    Vector vector = sijson_from_json(Vector, "{\"values\":[1,2]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);
    test_flt(vector.values[3], 0.0);

    vector = sijson_from_json(Vector, "{\"values\":[1,2,3,4,5]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);
    sireflect_fini();
}

void typed_array_rejects_wrong_type(void) {
    sireflect_init();
    Vector vector = sijson_from_json(Vector, "{\"values\":[1,\"bad\",3,4]}");
    test_not_null((void *)sijson_error());
    test_flt(vector.values[0], 0.0);

    Matrix matrix = sijson_from_json(Matrix, "{\"values\":[[1,2],[3]]}");
    test_not_null((void *)sijson_error());
    test_flt(matrix.values[0][0], 0.0);
    sireflect_fini();
}
