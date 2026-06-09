#include "sijson_internal.h"

char *sijson_dup_range(const char *start, size_t len) {
    char *result = malloc(len + 1);
    if (result == NULL) {
        sijson_set_error("out of memory");
        return NULL;
    }

    memcpy(result, start, len);
    result[len] = '\0';
    return result;
}

char *sijson_dup_cstr(const char *str) {
    if (str == NULL) {
        return NULL;
    }

    return sijson_dup_range(str, strlen(str));
}

bool sijson_reserve_array(sijson_array_t *array, size_t need) {
    if (array->cap >= need) {
        return true;
    }

    size_t cap = array->cap != 0 ? array->cap * 2 : 8;
    while (cap < need) {
        cap *= 2;
    }

    sijson_value_t *items = realloc(array->items, cap * sizeof(*items));
    if (items == NULL) {
        return sijson_set_error("out of memory");
    }

    array->items = items;
    array->cap = cap;
    return true;
}

bool sijson_reserve_object(sijson_object_t *object, size_t need) {
    if (object->cap >= need) {
        return true;
    }

    size_t cap = object->cap != 0 ? object->cap * 2 : 8;
    while (cap < need) {
        cap *= 2;
    }

    sijson_member_t *items = realloc(object->items, cap * sizeof(*items));
    if (items == NULL) {
        return sijson_set_error("out of memory");
    }

    object->items = items;
    object->cap = cap;
    return true;
}

sijson_value_t sijson_new_value(sijson_type_t type) {
    sijson_value_t value = calloc(1, sizeof(*value));
    if (value == NULL) {
        sijson_set_error("out of memory");
        return NULL;
    }

    value->type = type;
    return value;
}

sijson_value_t sijson_make_null(void) {
    sijson_clear_error();
    return sijson_new_value(SIJSON_NULL);
}

sijson_value_t sijson_make_bool(bool value) {
    sijson_clear_error();
    sijson_value_t result = sijson_new_value(SIJSON_BOOL);
    if (result != NULL) {
        result->as.boolean = value;
    }
    return result;
}

sijson_value_t sijson_make_number(double value) {
    sijson_clear_error();
    sijson_value_t result = sijson_new_value(SIJSON_NUMBER);
    if (result != NULL) {
        result->as.number = value;
    }
    return result;
}

sijson_value_t sijson_make_string(const char *value) {
    sijson_clear_error();
    sijson_value_t result = sijson_new_value(SIJSON_STRING);
    if (result == NULL) {
        return NULL;
    }

    result->as.string = sijson_dup_cstr(value != NULL ? value : "");
    if (result->as.string == NULL) {
        free(result);
        return NULL;
    }

    return result;
}

sijson_value_t sijson_make_array(void) {
    sijson_clear_error();
    return sijson_new_value(SIJSON_ARRAY);
}

sijson_value_t sijson_make_object(void) {
    sijson_clear_error();
    return sijson_new_value(SIJSON_OBJECT);
}

sijson_type_t sijson_type(sijson_value_t value) {
    return value != NULL ? value->type : SIJSON_NULL;
}

bool sijson_bool(sijson_value_t value) {
    return value != NULL && value->type == SIJSON_BOOL ? value->as.boolean : false;
}

double sijson_number(sijson_value_t value) {
    return value != NULL && value->type == SIJSON_NUMBER ? value->as.number : 0.0;
}

const char *sijson_string(sijson_value_t value) {
    return value != NULL && value->type == SIJSON_STRING ? value->as.string : NULL;
}

size_t sijson_array_len(sijson_value_t value) {
    return value != NULL && value->type == SIJSON_ARRAY ? value->as.array.len : 0;
}

sijson_value_t sijson_array_get(sijson_value_t value, size_t index) {
    if (value == NULL || value->type != SIJSON_ARRAY || index >= value->as.array.len) {
        return NULL;
    }

    return value->as.array.items[index];
}

size_t sijson_object_len(sijson_value_t value) {
    return value != NULL && value->type == SIJSON_OBJECT ? value->as.object.len : 0;
}

const char *sijson_object_key(sijson_value_t value, size_t index) {
    if (value == NULL || value->type != SIJSON_OBJECT || index >= value->as.object.len) {
        return NULL;
    }

    return value->as.object.items[index].key;
}

sijson_value_t sijson_object_get(sijson_value_t value, const char *key) {
    if (value == NULL || value->type != SIJSON_OBJECT || key == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < value->as.object.len; i++) {
        if (strcmp(value->as.object.items[i].key, key) == 0) {
            return value->as.object.items[i].value;
        }
    }

    return NULL;
}

bool sijson_array_push(sijson_value_t array, sijson_value_t value) {
    sijson_clear_error();
    if (array == NULL || array->type != SIJSON_ARRAY) {
        return sijson_set_error("sijson_array_push expects an array");
    }

    if (!sijson_reserve_array(&array->as.array, array->as.array.len + 1)) {
        return false;
    }

    array->as.array.items[array->as.array.len++] = value;
    return true;
}

bool sijson_object_set(sijson_value_t object, const char *key, sijson_value_t value) {
    sijson_clear_error();
    if (object == NULL || object->type != SIJSON_OBJECT) {
        return sijson_set_error("sijson_object_set expects an object");
    }
    if (key == NULL) {
        return sijson_set_error("sijson_object_set expects a key");
    }

    for (size_t i = 0; i < object->as.object.len; i++) {
        if (strcmp(object->as.object.items[i].key, key) == 0) {
            object->as.object.items[i].value = value;
            return true;
        }
    }

    if (!sijson_reserve_object(&object->as.object, object->as.object.len + 1)) {
        return false;
    }

    char *owned_key = sijson_dup_cstr(key);
    if (owned_key == NULL) {
        return false;
    }

    object->as.object.items[object->as.object.len++] = (sijson_member_t){
        .key = owned_key,
        .value = value,
    };
    return true;
}
