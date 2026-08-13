#include "sijson_internal.h"
#include "sireflect.h"

#include <limits.h>

static void *g_from_json_buffer;
static size_t g_from_json_capacity;

static sireflect_handle_t
sijson_register_type(sireflect_handle_t *ref, const sireflect_struct_desc_t *desc) {
    if (ref == NULL || desc == NULL) {
        sijson_set_error("missing reflection descriptor");
        return SIREFLECT_INVALID_HANDLE;
    }
    static const sireflect_struct_desc_t value_desc = {
        .name = "sijson_value_t",
        .fields = "{ ptr value; }",
        .size = sizeof(sijson_value_t),
        .align = _Alignof(sijson_value_t),
    };
    sireflect_register_struct(&value_desc);
    *ref = sireflect_register_struct(desc);
    return *ref;
}

static bool sijson_is_value_type(const sireflect_type_info_t *type) {
    return type != NULL && strcmp(type->name, "sijson_value_t") == 0 &&
           type->size == sizeof(sijson_value_t);
}

static bool sijson_is_char_pointer_type(const sireflect_type_info_t *type) {
    if (type == NULL) {
        return false;
    }
    if (type->kind == sireflect_kind_ptr) {
        return true;
    }
    if (type->kind != sireflect_kind_pointer) {
        return false;
    }

    const sireflect_type_info_t *pointee = sireflect_type_info(type->element_type);
    return pointee != NULL && pointee->kind == sireflect_kind_char;
}

static bool
sijson_write_reflected(sijson_writer_t *writer, sireflect_handle_t type, const void *ptr);

static bool sijson_write_reflected_field(
    sijson_writer_t *writer,
    const sireflect_type_info_t *field_type,
    const void *field_ptr
);

static bool sijson_write_reflected_array(
    sijson_writer_t *writer,
    const sireflect_type_info_t *array_type,
    const void *array_ptr
) {
    if (array_type == NULL || array_type->kind != sireflect_kind_array) {
        return sijson_set_error("expected reflected array type");
    }

    const sireflect_type_info_t *element_type =
        sireflect_type_info(array_type->element_type);
    if (element_type == NULL || element_type->size == 0) {
        return sijson_set_error("missing reflected array element type");
    }

    if (!sijson_writer_putc(writer, '[')) {
        return false;
    }

    for (size_t i = 0; i < array_type->element_count; i++) {
        const void *element_ptr = (const unsigned char *)array_ptr + i * element_type->size;

        if (i != 0 && !sijson_writer_putc(writer, ',')) {
            return false;
        }
        if (!sijson_write_reflected_field(writer, element_type, element_ptr)) {
            return false;
        }
    }

    return sijson_writer_putc(writer, ']');
}

static bool sijson_write_reflected_field(
    sijson_writer_t *writer,
    const sireflect_type_info_t *field_type,
    const void *field_ptr
) {
    if (field_type == NULL) {
        return sijson_set_error("missing reflected field type");
    }

    switch (field_type->kind) {
    case sireflect_kind_bool:
        return sijson_writer_cstr(writer, *(const bool *)field_ptr ? "true" : "false");
    default:
        break;
    }

    char number[64];
    switch (field_type->kind) {
    case sireflect_kind_signed_char:
        snprintf(number, sizeof(number), "%d", (int)*(const signed char *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_unsigned_char:
        snprintf(number, sizeof(number), "%u", (unsigned)*(const unsigned char *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_u8:
        snprintf(number, sizeof(number), "%u", (unsigned)*(const u8 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_u16:
        snprintf(number, sizeof(number), "%u", (unsigned)*(const u16 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_unsigned_short:
        snprintf(number, sizeof(number), "%u", (unsigned)*(const unsigned short *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_u32:
        snprintf(number, sizeof(number), "%u", *(const u32 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_unsigned_int:
        snprintf(number, sizeof(number), "%u", *(const unsigned int *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_u64:
        snprintf(number, sizeof(number), "%llu", (unsigned long long)*(const u64 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_i8:
        snprintf(number, sizeof(number), "%d", (int)*(const i8 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_i16:
        snprintf(number, sizeof(number), "%d", (int)*(const i16 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_i32:
        snprintf(number, sizeof(number), "%d", *(const i32 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_i64:
        snprintf(number, sizeof(number), "%lld", (long long)*(const i64 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_short:
        snprintf(number, sizeof(number), "%d", (int)*(const short *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_int:
        snprintf(number, sizeof(number), "%d", *(const int *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_long:
        snprintf(number, sizeof(number), "%ld", *(const long *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_unsigned_long:
        snprintf(number, sizeof(number), "%lu", *(const unsigned long *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_long_long:
        snprintf(number, sizeof(number), "%lld", *(const long long *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_unsigned_long_long:
        snprintf(number, sizeof(number), "%llu", *(const unsigned long long *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_f32:
        snprintf(number, sizeof(number), "%.9g", (double)*(const f32 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_f64:
        snprintf(number, sizeof(number), "%.17g", *(const f64 *)field_ptr);
        return sijson_writer_cstr(writer, number);
    case sireflect_kind_char:
        return sijson_writer_string(writer, (char[2]){ *(const char *)field_ptr, '\0' });
    case sireflect_kind_ptr:
        return sijson_writer_string(writer, *(char *const *)field_ptr);
    case sireflect_kind_pointer:
        if (sijson_is_char_pointer_type(field_type)) {
            return sijson_writer_string(writer, *(char *const *)field_ptr);
        }
        break;
    case sireflect_kind_struct:
        if (sijson_is_value_type(field_type)) {
            return sijson_write_value(writer, *(const sijson_value_t *)field_ptr);
        }
        return sijson_write_reflected(
            writer,
            sireflect_type_by_name(field_type->name),
            field_ptr
        );
    case sireflect_kind_array:
        return sijson_write_reflected_array(writer, field_type, field_ptr);
    case sireflect_kind_bool:
        break;
    default:
        break;
    }

    return sijson_set_error("unsupported field type for serialization");
}

static bool
sijson_write_reflected(sijson_writer_t *writer, sireflect_handle_t type, const void *ptr) {
    const sireflect_type_info_t *info = sireflect_type_info(type);
    if (info == NULL || info->kind != sireflect_kind_struct) {
        return sijson_set_error("expected reflected struct");
    }

    if (!sijson_writer_putc(writer, '{')) {
        return false;
    }

    const sireflect_fields_t *fields = &info->fields;
    for (size_t i = 0; i < fields->field_count; i++) {
        const sireflect_field_info_t *field = &fields->fields[i];
        const sireflect_type_info_t *field_type = sireflect_type_info(field->type);
        const void *field_ptr = (const unsigned char *)ptr + field->offset;

        if (i != 0 && !sijson_writer_putc(writer, ',')) {
            return false;
        }
        if (!sijson_writer_string(writer, field->name) || !sijson_writer_putc(writer, ':')) {
            return false;
        }
        if (!sijson_write_reflected_field(writer, field_type, field_ptr)) {
            return false;
        }
    }

    return sijson_writer_putc(writer, '}');
}

char *
sijson_to_json_impl(sireflect_handle_t *ref, const sireflect_struct_desc_t *desc, const void *ptr) {
    sijson_clear_error();
    if (ptr == NULL) {
        sijson_set_error("sijson_to_json expects a value pointer");
        return NULL;
    }

    sireflect_handle_t type = sijson_register_type(ref, desc);
    if (type == SIREFLECT_INVALID_HANDLE) {
        return NULL;
    }

    sijson_writer_t writer = { 0 };
    if (!sijson_write_reflected(&writer, type, ptr)) {
        free(writer.data);
        return NULL;
    }
    return writer.data;
}

static bool sijson_number_is_integer(double value) {
    if (!isfinite(value) || value < -9007199254740991.0 || value > 9007199254740991.0) {
        return false;
    }

    int64_t integer = (int64_t)value;
    return (double)integer == value;
}

static bool sijson_assign_number(
    const sireflect_type_info_t *field_type,
    void *field_ptr,
    sijson_value_t value
) {
    if (value == NULL || value->type != SIJSON_NUMBER) {
        return sijson_set_error("expected JSON number");
    }

    double number = value->as.number;
    switch (field_type->kind) {
    case sireflect_kind_signed_char:
        if (!sijson_number_is_integer(number) || number < SCHAR_MIN || number > SCHAR_MAX) {
            return sijson_set_error("number out of range for signed char");
        }
        *(signed char *)field_ptr = (signed char)number;
        return true;
    case sireflect_kind_unsigned_char:
        if (!sijson_number_is_integer(number) || number < 0 || number > UCHAR_MAX) {
            return sijson_set_error("number out of range for unsigned char");
        }
        *(unsigned char *)field_ptr = (unsigned char)number;
        return true;
    case sireflect_kind_u8:
        if (!sijson_number_is_integer(number) || number < 0 || number > UINT8_MAX) {
            return sijson_set_error("number out of range for u8");
        }
        *(u8 *)field_ptr = (u8)number;
        return true;
    case sireflect_kind_u16:
        if (!sijson_number_is_integer(number) || number < 0 || number > UINT16_MAX) {
            return sijson_set_error("number out of range for u16");
        }
        *(u16 *)field_ptr = (u16)number;
        return true;
    case sireflect_kind_unsigned_short:
        if (!sijson_number_is_integer(number) || number < 0 || number > USHRT_MAX) {
            return sijson_set_error("number out of range for unsigned short");
        }
        *(unsigned short *)field_ptr = (unsigned short)number;
        return true;
    case sireflect_kind_u32:
        if (!sijson_number_is_integer(number) || number < 0 || number > UINT32_MAX) {
            return sijson_set_error("number out of range for u32");
        }
        *(u32 *)field_ptr = (u32)number;
        return true;
    case sireflect_kind_unsigned_int:
        if (!sijson_number_is_integer(number) || number < 0 || number > UINT_MAX) {
            return sijson_set_error("number out of range for unsigned int");
        }
        *(unsigned int *)field_ptr = (unsigned int)number;
        return true;
    case sireflect_kind_u64:
        if (!sijson_number_is_integer(number) || number < 0) {
            return sijson_set_error("number out of range for u64");
        }
        *(u64 *)field_ptr = (u64)number;
        return true;
    case sireflect_kind_i8:
        if (!sijson_number_is_integer(number) || number < INT8_MIN || number > INT8_MAX) {
            return sijson_set_error("number out of range for i8");
        }
        *(i8 *)field_ptr = (i8)number;
        return true;
    case sireflect_kind_i16:
        if (!sijson_number_is_integer(number) || number < INT16_MIN || number > INT16_MAX) {
            return sijson_set_error("number out of range for i16");
        }
        *(i16 *)field_ptr = (i16)number;
        return true;
    case sireflect_kind_i32:
        if (!sijson_number_is_integer(number) || number < INT32_MIN || number > INT32_MAX) {
            return sijson_set_error("number out of range for i32");
        }
        *(i32 *)field_ptr = (i32)number;
        return true;
    case sireflect_kind_i64:
        if (!sijson_number_is_integer(number)) {
            return sijson_set_error("number out of range for i64");
        }
        *(i64 *)field_ptr = (i64)number;
        return true;
    case sireflect_kind_short:
        if (!sijson_number_is_integer(number)) {
            return sijson_set_error("expected integer for short");
        }
        *(short *)field_ptr = (short)number;
        return true;
    case sireflect_kind_int:
        if (!sijson_number_is_integer(number)) {
            return sijson_set_error("expected integer for int");
        }
        *(int *)field_ptr = (int)number;
        return true;
    case sireflect_kind_long:
        if (!sijson_number_is_integer(number)) {
            return sijson_set_error("expected integer for long");
        }
        *(long *)field_ptr = (long)number;
        return true;
    case sireflect_kind_unsigned_long:
        if (!sijson_number_is_integer(number) || number < 0) {
            return sijson_set_error("number out of range for unsigned long");
        }
        *(unsigned long *)field_ptr = (unsigned long)number;
        return true;
    case sireflect_kind_long_long:
        if (!sijson_number_is_integer(number)) {
            return sijson_set_error("number out of range for long long");
        }
        *(long long *)field_ptr = (long long)number;
        return true;
    case sireflect_kind_unsigned_long_long:
        if (!sijson_number_is_integer(number) || number < 0) {
            return sijson_set_error("number out of range for unsigned long long");
        }
        *(unsigned long long *)field_ptr = (unsigned long long)number;
        return true;
    case sireflect_kind_f32:
        *(f32 *)field_ptr = (f32)number;
        return true;
    case sireflect_kind_f64:
        *(f64 *)field_ptr = number;
        return true;
    default:
        return sijson_set_error("field is not numeric");
    }
}

static bool sijson_assign_reflected(sireflect_handle_t type, void *ptr, sijson_value_t value);

static bool
sijson_assign_field(const sireflect_type_info_t *field_type, void *field_ptr, sijson_value_t value);

static bool sijson_assign_array(
    const sireflect_type_info_t *array_type,
    void *array_ptr,
    sijson_value_t value
) {
    if (array_type == NULL || array_type->kind != sireflect_kind_array) {
        return sijson_set_error("expected reflected array type");
    }
    if (value == NULL || value->type != SIJSON_ARRAY) {
        return sijson_set_error("expected JSON array");
    }
    if (value->as.array.len != array_type->element_count) {
        return sijson_set_error("JSON array length does not match reflected array");
    }

    const sireflect_type_info_t *element_type =
        sireflect_type_info(array_type->element_type);
    if (element_type == NULL || element_type->size == 0) {
        return sijson_set_error("missing reflected array element type");
    }

    for (size_t i = 0; i < array_type->element_count; i++) {
        void *element_ptr = (unsigned char *)array_ptr + i * element_type->size;
        if (!sijson_assign_field(element_type, element_ptr, value->as.array.items[i])) {
            return false;
        }
    }

    return true;
}

static bool sijson_assign_field(
    const sireflect_type_info_t *field_type,
    void *field_ptr,
    sijson_value_t value
) {
    if (field_type == NULL) {
        return sijson_set_error("missing reflected field type");
    }

    switch (field_type->kind) {
    case sireflect_kind_bool:
        if (value == NULL || value->type != SIJSON_BOOL) {
            return sijson_set_error("expected JSON bool");
        }
        *(bool *)field_ptr = value->as.boolean;
        return true;
    case sireflect_kind_signed_char:
    case sireflect_kind_unsigned_char:
    case sireflect_kind_u8:
    case sireflect_kind_u16:
    case sireflect_kind_u32:
    case sireflect_kind_u64:
    case sireflect_kind_i8:
    case sireflect_kind_i16:
    case sireflect_kind_i32:
    case sireflect_kind_i64:
    case sireflect_kind_unsigned_short:
    case sireflect_kind_short:
    case sireflect_kind_unsigned_int:
    case sireflect_kind_int:
    case sireflect_kind_unsigned_long:
    case sireflect_kind_long:
    case sireflect_kind_long_long:
    case sireflect_kind_unsigned_long_long:
    case sireflect_kind_f32:
    case sireflect_kind_f64:
        return sijson_assign_number(field_type, field_ptr, value);
    case sireflect_kind_char:
        if (value == NULL || value->type != SIJSON_STRING || value->as.string[0] == '\0') {
            return sijson_set_error("expected non-empty JSON string for char");
        }
        *(char *)field_ptr = value->as.string[0];
        return true;
    case sireflect_kind_ptr:
    case sireflect_kind_pointer:
        if (!sijson_is_char_pointer_type(field_type)) {
            break;
        }
        if (value == NULL || value->type == SIJSON_NULL) {
            *(char **)field_ptr = NULL;
            return true;
        }
        if (value->type != SIJSON_STRING) {
            return sijson_set_error("expected JSON string for pointer field");
        }
        *(char **)field_ptr = sijson_dup_cstr(value->as.string);
        return *(char **)field_ptr != NULL;
    case sireflect_kind_struct:
        if (sijson_is_value_type(field_type)) {
            *(sijson_value_t *)field_ptr = value;
            return true;
        }
        return sijson_assign_reflected(
            sireflect_type_by_name(field_type->name),
            field_ptr,
            value
        );
    case sireflect_kind_array:
        return sijson_assign_array(field_type, field_ptr, value);
    default:
        break;
    }

    return sijson_set_error("unsupported field type for deserialization");
}

static bool sijson_assign_reflected(sireflect_handle_t type, void *ptr, sijson_value_t value) {
    if (value == NULL || value->type != SIJSON_OBJECT) {
        return sijson_set_error("expected JSON object for reflected struct");
    }

    const sireflect_type_info_t *info = sireflect_type_info(type);
    if (info == NULL || info->kind != sireflect_kind_struct) {
        return sijson_set_error("expected reflected struct type");
    }

    memset(ptr, 0, info->size);
    const sireflect_fields_t *fields = &info->fields;
    for (size_t i = 0; i < fields->field_count; i++) {
        const sireflect_field_info_t *field = &fields->fields[i];
        sijson_value_t member = sijson_object_get(value, field->name);
        if (member == NULL) {
            continue;
        }

        const sireflect_type_info_t *field_type = sireflect_type_info(field->type);
        void *field_ptr = (unsigned char *)ptr + field->offset;
        if (!sijson_assign_field(field_type, field_ptr, member)) {
            return false;
        }
    }

    return true;
}

void *sijson_from_json_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    const char *json
) {
    sijson_clear_error();
    sireflect_handle_t type = sijson_register_type(ref, desc);
    if (type == SIREFLECT_INVALID_HANDLE) {
        return NULL;
    }

    const sireflect_type_info_t *info = sireflect_type_info(type);
    if (info == NULL) {
        sijson_set_error("missing reflected type info");
        return NULL;
    }

    if (g_from_json_capacity < info->size) {
        void *buffer = realloc(g_from_json_buffer, info->size);
        if (buffer == NULL) {
            sijson_set_error("out of memory");
            return NULL;
        }
        g_from_json_buffer = buffer;
        g_from_json_capacity = info->size;
    }

    memset(g_from_json_buffer, 0, info->size);
    sijson_value_t root = sijson_parse(json);
    if (root == NULL) {
        memset(g_from_json_buffer, 0, info->size);
        return g_from_json_buffer;
    }

    if (!sijson_assign_reflected(type, g_from_json_buffer, root)) {
        memset(g_from_json_buffer, 0, info->size);
        return g_from_json_buffer;
    }

    return g_from_json_buffer;
}

static void sijson_free_reflected_field(const sireflect_type_info_t *field_type, void *field_ptr);

static void sijson_free_reflected(sireflect_handle_t type, void *ptr) {
    if (ptr == NULL) {
        return;
    }

    const sireflect_type_info_t *info = sireflect_type_info(type);
    if (info == NULL || info->kind != sireflect_kind_struct || sijson_is_value_type(info)) {
        return;
    }

    const sireflect_fields_t *fields = &info->fields;
    for (size_t i = 0; i < fields->field_count; i++) {
        const sireflect_field_info_t *field = &fields->fields[i];
        const sireflect_type_info_t *field_type = sireflect_type_info(field->type);
        void *field_ptr = (unsigned char *)ptr + field->offset;

        sijson_free_reflected_field(field_type, field_ptr);
    }
}

static void sijson_free_reflected_field(const sireflect_type_info_t *field_type, void *field_ptr) {
    if (field_type == NULL || field_ptr == NULL) {
        return;
    }

    switch (field_type->kind) {
    case sireflect_kind_ptr:
        free(*(void **)field_ptr);
        *(void **)field_ptr = NULL;
        return;
    case sireflect_kind_pointer:
        if (sijson_is_char_pointer_type(field_type)) {
            free(*(void **)field_ptr);
            *(void **)field_ptr = NULL;
        }
        return;
    case sireflect_kind_struct:
        if (!sijson_is_value_type(field_type)) {
            sijson_free_reflected(sireflect_type_by_name(field_type->name), field_ptr);
        }
        return;
    case sireflect_kind_array: {
        const sireflect_type_info_t *element_type =
            sireflect_type_info(field_type->element_type);
        if (element_type == NULL || element_type->size == 0) {
            return;
        }
        for (size_t i = 0; i < field_type->element_count; i++) {
            void *element_ptr = (unsigned char *)field_ptr + i * element_type->size;
            sijson_free_reflected_field(element_type, element_ptr);
        }
        return;
    }
    case sireflect_kind_u8:
    case sireflect_kind_u16:
    case sireflect_kind_u32:
    case sireflect_kind_u64:
    case sireflect_kind_i8:
    case sireflect_kind_i16:
    case sireflect_kind_i32:
    case sireflect_kind_i64:
    case sireflect_kind_signed_char:
    case sireflect_kind_unsigned_char:
    case sireflect_kind_unsigned_short:
    case sireflect_kind_unsigned_int:
    case sireflect_kind_unsigned_long:
    case sireflect_kind_long_long:
    case sireflect_kind_unsigned_long_long:
    case sireflect_kind_f32:
    case sireflect_kind_f64:
    case sireflect_kind_bool:
    case sireflect_kind_char:
    case sireflect_kind_short:
    case sireflect_kind_int:
    case sireflect_kind_long:
    case sireflect_kind_function_pointer:
        return;
    }
}

void sijson_free_impl(sireflect_handle_t *ref, const sireflect_struct_desc_t *desc, void *ptr) {
    sijson_clear_error();
    sireflect_handle_t type = sijson_register_type(ref, desc);
    if (type == SIREFLECT_INVALID_HANDLE) {
        return;
    }
    sijson_free_reflected(type, ptr);
}
