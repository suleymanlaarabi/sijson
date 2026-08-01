#include "sijson_internal.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifndef SIJSON_ARENA_RESERVE
#define SIJSON_ARENA_RESERVE ((size_t)1 << 30)
#endif

#ifndef SIJSON_ARENA_FALLBACK_RESERVE
#define SIJSON_ARENA_FALLBACK_RESERVE ((size_t)1 << 20)
#endif

typedef struct sijson_arena {
    unsigned char *data;
    size_t used;
    size_t cap;
    size_t reserve;
    bool mmap_backed;
} sijson_arena_t;

static sijson_arena_t g_arena;

static size_t sijson_align_forward(size_t value, size_t align) {
    size_t mask = align - 1;
    return (value + mask) & ~mask;
}

static size_t sijson_page_size(void) {
#if defined(__unix__) || defined(__APPLE__)
    long page = sysconf(_SC_PAGESIZE);
    if (page > 0) {
        return (size_t)page;
    }
#endif
    return 4096;
}

static bool sijson_arena_init(size_t need) {
    if (g_arena.data != NULL) {
        return true;
    }

    size_t page_size = sijson_page_size();
    size_t reserve = SIJSON_ARENA_RESERVE;
    if (reserve < need) {
        reserve = sijson_align_forward(need, page_size);
    }

#if defined(__unix__) || defined(__APPLE__)
    void *data = mmap(NULL, reserve, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (data != MAP_FAILED) {
        g_arena.data = data;
        g_arena.reserve = reserve;
        g_arena.mmap_backed = true;
        return true;
    }
#endif

    size_t fallback_reserve = reserve;
    if (need <= SIJSON_ARENA_FALLBACK_RESERVE && fallback_reserve > SIJSON_ARENA_FALLBACK_RESERVE) {
        fallback_reserve = SIJSON_ARENA_FALLBACK_RESERVE;
    }

    g_arena.data = malloc(fallback_reserve);
    if (g_arena.data == NULL) {
        sijson_set_error("out of memory");
        return false;
    }
    g_arena.cap = fallback_reserve;
    g_arena.reserve = fallback_reserve;
    return true;
}

static bool sijson_arena_commit(size_t need) {
    if (!sijson_arena_init(need)) {
        return false;
    }
    if (need <= g_arena.cap) {
        return true;
    }
    if (need > g_arena.reserve) {
        return sijson_set_error("sijson arena capacity exceeded");
    }

    size_t page_size = sijson_page_size();
    size_t cap = sijson_align_forward(need, page_size);

    if (g_arena.mmap_backed) {
#if defined(__unix__) || defined(__APPLE__)
        if (mprotect(g_arena.data + g_arena.cap, cap - g_arena.cap, PROT_READ | PROT_WRITE) != 0) {
            return sijson_set_error("out of memory");
        }
#else
        return sijson_set_error("sijson arena backend unavailable");
#endif
    }

    g_arena.cap = cap;
    return true;
}

void *sijson_arena_alloc(size_t size, size_t align) {
    if (align == 0) {
        align = _Alignof(sijson_max_align_t);
    }

    size_t offset = sijson_align_forward(g_arena.used, align);
    if (offset < g_arena.used || size > SIZE_MAX - offset) {
        sijson_set_error("out of memory");
        return NULL;
    }

    size_t need = offset + size;
    if (!sijson_arena_commit(need)) {
        return NULL;
    }

    void *ptr = g_arena.data + offset;
    g_arena.used = need;
    return ptr;
}

char *sijson_arena_dup_range(const char *start, size_t len) {
    char *result = sijson_arena_alloc(len + 1, _Alignof(char));
    if (result == NULL) {
        return NULL;
    }

    memcpy(result, start, len);
    result[len] = '\0';
    return result;
}

char *sijson_arena_dup_cstr(const char *str) {
    if (str == NULL) {
        return NULL;
    }

    return sijson_arena_dup_range(str, strlen(str));
}

size_t sijson_arena_mark(void) {
    return g_arena.used;
}

void sijson_arena_rewind(size_t mark) {
    if (mark <= g_arena.used) {
        g_arena.used = mark;
    }
}

void sijson_clean(void) {
    sijson_clear_error();
    g_arena.used = 0;
}

void sijson_release(void) {
    sijson_clear_error();
    if (g_arena.mmap_backed) {
#if defined(__unix__) || defined(__APPLE__)
        munmap(g_arena.data, g_arena.reserve);
#endif
    } else {
        free(g_arena.data);
    }
    g_arena = (sijson_arena_t){ 0 };
}

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

    sijson_value_t *items = sijson_arena_alloc(cap * sizeof(*items), _Alignof(sijson_value_t));
    if (items == NULL) {
        return false;
    }
    if (array->items != NULL) {
        memcpy(items, array->items, array->len * sizeof(*items));
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

    sijson_member_t *items = sijson_arena_alloc(cap * sizeof(*items), _Alignof(sijson_member_t));
    if (items == NULL) {
        return false;
    }
    if (object->items != NULL) {
        memcpy(items, object->items, object->len * sizeof(*items));
    }

    object->items = items;
    object->cap = cap;
    return true;
}

sijson_value_t sijson_new_value(sijson_type_t type) {
    sijson_value_t value = sijson_arena_alloc(sizeof(*value), _Alignof(struct sijson_value));
    if (value == NULL) {
        return NULL;
    }

    memset(value, 0, sizeof(*value));
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

    result->as.string = sijson_arena_dup_cstr(value != NULL ? value : "");
    if (result->as.string == NULL) {
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

    char *owned_key = sijson_arena_dup_cstr(key);
    if (owned_key == NULL) {
        return false;
    }

    object->as.object.items[object->as.object.len++] = (sijson_member_t){
        .key = owned_key,
        .value = value,
    };
    return true;
}
