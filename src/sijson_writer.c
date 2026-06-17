#include "sijson_internal.h"

bool sijson_writer_reserve(sijson_writer_t *writer, size_t extra) {
    if (writer->len + extra + 1 <= writer->cap) {
        return true;
    }

    size_t cap = writer->cap != 0 ? writer->cap * 2 : 128;
    while (cap < writer->len + extra + 1) {
        cap *= 2;
    }

    char *data = realloc(writer->data, cap);
    if (data == NULL) {
        return sijson_set_error("out of memory");
    }

    writer->data = data;
    writer->cap = cap;
    return true;
}

bool sijson_writer_putc(sijson_writer_t *writer, char c) {
    if (!sijson_writer_reserve(writer, 1)) {
        return false;
    }

    writer->data[writer->len++] = c;
    writer->data[writer->len] = '\0';
    return true;
}

bool sijson_writer_write(sijson_writer_t *writer, const char *data, size_t len) {
    if (!sijson_writer_reserve(writer, len)) {
        return false;
    }

    memcpy(writer->data + writer->len, data, len);
    writer->len += len;
    writer->data[writer->len] = '\0';
    return true;
}

bool sijson_writer_cstr(sijson_writer_t *writer, const char *str) {
    return sijson_writer_write(writer, str, strlen(str));
}

bool sijson_writer_string(sijson_writer_t *writer, const char *str) {
    if (str == NULL) {
        return sijson_writer_cstr(writer, "null");
    }

    if (!sijson_writer_putc(writer, '"')) {
        return false;
    }

    const unsigned char *cur = (const unsigned char *)str;
    const unsigned char *chunk = cur;
    while (*cur != '\0') {
        unsigned char c = *cur;
        if (c == '"' || c == '\\' || c < 0x20) {
            if (!sijson_writer_write(writer, (const char *)chunk, (size_t)(cur - chunk))) {
                return false;
            }

            switch (c) {
            case '"':
                if (!sijson_writer_cstr(writer, "\\\"")) {
                    return false;
                }
                break;
            case '\\':
                if (!sijson_writer_cstr(writer, "\\\\")) {
                    return false;
                }
                break;
            case '\b':
                if (!sijson_writer_cstr(writer, "\\b")) {
                    return false;
                }
                break;
            case '\f':
                if (!sijson_writer_cstr(writer, "\\f")) {
                    return false;
                }
                break;
            case '\n':
                if (!sijson_writer_cstr(writer, "\\n")) {
                    return false;
                }
                break;
            case '\r':
                if (!sijson_writer_cstr(writer, "\\r")) {
                    return false;
                }
                break;
            case '\t':
                if (!sijson_writer_cstr(writer, "\\t")) {
                    return false;
                }
                break;
            default: {
                char escape[7];
                snprintf(escape, sizeof(escape), "\\u%04x", c);
                if (!sijson_writer_cstr(writer, escape)) {
                    return false;
                }
                break;
            }
            }

            cur++;
            chunk = cur;
            continue;
        }
        cur++;
    }

    if (!sijson_writer_write(writer, (const char *)chunk, (size_t)(cur - chunk))) {
        return false;
    }

    return sijson_writer_putc(writer, '"');
}

bool sijson_write_value(sijson_writer_t *writer, sijson_value_t value);

static bool sijson_write_array(sijson_writer_t *writer, sijson_value_t value) {
    if (!sijson_writer_putc(writer, '[')) {
        return false;
    }

    for (size_t i = 0; i < value->as.array.len; i++) {
        if (i != 0 && !sijson_writer_putc(writer, ',')) {
            return false;
        }
        if (!sijson_write_value(writer, value->as.array.items[i])) {
            return false;
        }
    }

    return sijson_writer_putc(writer, ']');
}

static bool sijson_write_object(sijson_writer_t *writer, sijson_value_t value) {
    if (!sijson_writer_putc(writer, '{')) {
        return false;
    }

    for (size_t i = 0; i < value->as.object.len; i++) {
        if (i != 0 && !sijson_writer_putc(writer, ',')) {
            return false;
        }
        if (!sijson_writer_string(writer, value->as.object.items[i].key)) {
            return false;
        }
        if (!sijson_writer_putc(writer, ':')) {
            return false;
        }
        if (!sijson_write_value(writer, value->as.object.items[i].value)) {
            return false;
        }
    }

    return sijson_writer_putc(writer, '}');
}

bool sijson_write_value(sijson_writer_t *writer, sijson_value_t value) {
    if (value == NULL) {
        return sijson_writer_cstr(writer, "null");
    }

    switch (value->type) {
    case SIJSON_NULL:
        return sijson_writer_cstr(writer, "null");
    case SIJSON_BOOL:
        return sijson_writer_cstr(writer, value->as.boolean ? "true" : "false");
    case SIJSON_NUMBER: {
        if (!isfinite(value->as.number)) {
            return sijson_set_error("cannot write non-finite JSON number");
        }
        char number[64];
        int len = snprintf(number, sizeof(number), "%.17g", value->as.number);
        if (len < 0 || (size_t)len >= sizeof(number)) {
            return sijson_set_error("failed to format JSON number");
        }
        return sijson_writer_write(writer, number, (size_t)len);
    }
    case SIJSON_STRING:
        return sijson_writer_string(writer, value->as.string);
    case SIJSON_ARRAY:
        return sijson_write_array(writer, value);
    case SIJSON_OBJECT:
        return sijson_write_object(writer, value);
    }

    return sijson_set_error("unknown JSON value type");
}

char *sijson_value_to_str(sijson_value_t value) {
    sijson_clear_error();
    sijson_writer_t writer = { 0 };
    if (!sijson_write_value(&writer, value)) {
        free(writer.data);
        return NULL;
    }
    if (writer.data == NULL) {
        return sijson_dup_cstr("");
    }
    return writer.data;
}

