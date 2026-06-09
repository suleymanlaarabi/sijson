#include "sijson_internal.h"

static void sijson_skip_ws(sijson_parser_t *parser) {
    while (isspace((unsigned char)*parser->cur)) {
        parser->cur++;
    }
}

static bool sijson_take(sijson_parser_t *parser, char c) {
    sijson_skip_ws(parser);
    if (*parser->cur != c) {
        return false;
    }
    parser->cur++;
    return true;
}

static int sijson_hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    return -1;
}

static bool sijson_writer_utf8(sijson_writer_t *writer, unsigned codepoint) {
    char out[4];
    size_t len = 0;

    if (codepoint <= 0x7f) {
        out[len++] = (char)codepoint;
    } else if (codepoint <= 0x7ff) {
        out[len++] = (char)(0xc0 | (codepoint >> 6));
        out[len++] = (char)(0x80 | (codepoint & 0x3f));
    } else {
        out[len++] = (char)(0xe0 | (codepoint >> 12));
        out[len++] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        out[len++] = (char)(0x80 | (codepoint & 0x3f));
    }

    return sijson_writer_write(writer, out, len);
}

static char *sijson_parse_string_raw(sijson_parser_t *parser) {
    sijson_skip_ws(parser);
    if (*parser->cur != '"') {
        sijson_set_error_at("expected JSON string", parser->cur);
        return NULL;
    }
    parser->cur++;

    sijson_writer_t writer = { 0 };
    const char *chunk = parser->cur;
    while (*parser->cur != '\0') {
        unsigned char c = (unsigned char)*parser->cur;
        if (c == '"') {
            if (!sijson_writer_write(&writer, chunk, (size_t)(parser->cur - chunk))) {
                free(writer.data);
                return NULL;
            }
            parser->cur++;
            if (writer.data == NULL) {
                return sijson_dup_cstr("");
            }
            return writer.data;
        }

        if (c < 0x20) {
            free(writer.data);
            sijson_set_error_at("control character in JSON string", parser->cur);
            return NULL;
        }

        if (c != '\\') {
            parser->cur++;
            continue;
        }

        if (!sijson_writer_write(&writer, chunk, (size_t)(parser->cur - chunk))) {
            free(writer.data);
            return NULL;
        }

        parser->cur++;
        char escaped = *parser->cur++;
        switch (escaped) {
        case '"':
        case '\\':
        case '/':
            if (!sijson_writer_putc(&writer, escaped)) {
                free(writer.data);
                return NULL;
            }
            break;
        case 'b':
            if (!sijson_writer_putc(&writer, '\b')) {
                free(writer.data);
                return NULL;
            }
            break;
        case 'f':
            if (!sijson_writer_putc(&writer, '\f')) {
                free(writer.data);
                return NULL;
            }
            break;
        case 'n':
            if (!sijson_writer_putc(&writer, '\n')) {
                free(writer.data);
                return NULL;
            }
            break;
        case 'r':
            if (!sijson_writer_putc(&writer, '\r')) {
                free(writer.data);
                return NULL;
            }
            break;
        case 't':
            if (!sijson_writer_putc(&writer, '\t')) {
                free(writer.data);
                return NULL;
            }
            break;
        case 'u': {
            unsigned codepoint = 0;
            for (size_t i = 0; i < 4; i++) {
                int digit = sijson_hex_digit(parser->cur[i]);
                if (digit < 0) {
                    free(writer.data);
                    sijson_set_error_at("invalid unicode escape", parser->cur);
                    return NULL;
                }
                codepoint = (codepoint << 4) | (unsigned)digit;
            }
            parser->cur += 4;
            if (!sijson_writer_utf8(&writer, codepoint)) {
                free(writer.data);
                return NULL;
            }
            break;
        }
        default:
            free(writer.data);
            sijson_set_error_at("invalid JSON string escape", parser->cur - 1);
            return NULL;
        }
        chunk = parser->cur;
    }

    free(writer.data);
    sijson_set_error("unterminated JSON string");
    return NULL;
}

static sijson_value_t sijson_parse_value(sijson_parser_t *parser);

static sijson_value_t sijson_parse_array_value(sijson_parser_t *parser) {
    if (!sijson_take(parser, '[')) {
        return NULL;
    }

    sijson_value_t array = sijson_new_value(SIJSON_ARRAY);
    if (array == NULL) {
        return NULL;
    }

    sijson_skip_ws(parser);
    if (*parser->cur == ']') {
        parser->cur++;
        return array;
    }

    for (;;) {
        sijson_value_t item = sijson_parse_value(parser);
        if (item == NULL) {
            return NULL;
        }
        if (!sijson_reserve_array(&array->as.array, array->as.array.len + 1)) {
            return NULL;
        }
        array->as.array.items[array->as.array.len++] = item;

        sijson_skip_ws(parser);
        if (*parser->cur == ']') {
            parser->cur++;
            return array;
        }
        if (*parser->cur != ',') {
            sijson_set_error_at("expected ',' or ']'", parser->cur);
            return NULL;
        }
        parser->cur++;
    }
}

static sijson_value_t sijson_parse_object_value(sijson_parser_t *parser) {
    if (!sijson_take(parser, '{')) {
        return NULL;
    }

    sijson_value_t object = sijson_new_value(SIJSON_OBJECT);
    if (object == NULL) {
        return NULL;
    }

    sijson_skip_ws(parser);
    if (*parser->cur == '}') {
        parser->cur++;
        return object;
    }

    for (;;) {
        char *key = sijson_parse_string_raw(parser);
        if (key == NULL) {
            return NULL;
        }
        if (!sijson_take(parser, ':')) {
            free(key);
            sijson_set_error_at("expected ':'", parser->cur);
            return NULL;
        }

        sijson_value_t item = sijson_parse_value(parser);
        if (item == NULL) {
            free(key);
            return NULL;
        }

        if (!sijson_reserve_object(&object->as.object, object->as.object.len + 1)) {
            free(key);
            return NULL;
        }
        object->as.object.items[object->as.object.len++] = (sijson_member_t){
            .key = key,
            .value = item,
        };

        sijson_skip_ws(parser);
        if (*parser->cur == '}') {
            parser->cur++;
            return object;
        }
        if (*parser->cur != ',') {
            sijson_set_error_at("expected ',' or '}'", parser->cur);
            return NULL;
        }
        parser->cur++;
    }
}

static sijson_value_t sijson_parse_number_value(sijson_parser_t *parser) {
    sijson_skip_ws(parser);
    const char *start = parser->cur;
    const char *scan = start;

    if (*scan == '-') {
        scan++;
    }

    if (*scan == '0') {
        scan++;
    } else if (*scan >= '1' && *scan <= '9') {
        do {
            scan++;
        } while (isdigit((unsigned char)*scan));
    } else {
        sijson_set_error_at("invalid JSON number", start);
        return NULL;
    }

    if (*scan == '.') {
        scan++;
        if (!isdigit((unsigned char)*scan)) {
            sijson_set_error_at("invalid JSON number", start);
            return NULL;
        }
        do {
            scan++;
        } while (isdigit((unsigned char)*scan));
    }

    if (*scan == 'e' || *scan == 'E') {
        scan++;
        if (*scan == '+' || *scan == '-') {
            scan++;
        }
        if (!isdigit((unsigned char)*scan)) {
            sijson_set_error_at("invalid JSON number", start);
            return NULL;
        }
        do {
            scan++;
        } while (isdigit((unsigned char)*scan));
    }

    size_t len = (size_t)(scan - start);
    char stack[128];
    char *number_text = stack;
    if (len >= sizeof(stack)) {
        number_text = malloc(len + 1);
        if (number_text == NULL) {
            sijson_set_error("out of memory");
            return NULL;
        }
    }
    memcpy(number_text, start, len);
    number_text[len] = '\0';

    errno = 0;
    char *end = NULL;
    double number = strtod(number_text, &end);
    if (end == number_text || *end != '\0' || errno == ERANGE || !isfinite(number)) {
        if (number_text != stack) {
            free(number_text);
        }
        sijson_set_error_at("invalid JSON number", start);
        return NULL;
    }
    if (number_text != stack) {
        free(number_text);
    }

    parser->cur = scan;
    sijson_value_t value = sijson_new_value(SIJSON_NUMBER);
    if (value != NULL) {
        value->as.number = number;
    }
    return value;
}

static sijson_value_t
sijson_parse_literal(sijson_parser_t *parser, const char *literal, sijson_value_t value) {
    size_t len = strlen(literal);
    if (strncmp(parser->cur, literal, len) != 0) {
        sijson_set_error_at("invalid JSON literal", parser->cur);
        return NULL;
    }
    parser->cur += len;
    return value;
}

static sijson_value_t sijson_parse_value(sijson_parser_t *parser) {
    sijson_skip_ws(parser);
    switch (*parser->cur) {
    case 'n':
        return sijson_parse_literal(parser, "null", sijson_new_value(SIJSON_NULL));
    case 't': {
        sijson_value_t value = sijson_new_value(SIJSON_BOOL);
        if (value != NULL) {
            value->as.boolean = true;
        }
        return sijson_parse_literal(parser, "true", value);
    }
    case 'f': {
        sijson_value_t value = sijson_new_value(SIJSON_BOOL);
        if (value != NULL) {
            value->as.boolean = false;
        }
        return sijson_parse_literal(parser, "false", value);
    }
    case '"': {
        char *string = sijson_parse_string_raw(parser);
        if (string == NULL) {
            return NULL;
        }
        sijson_value_t value = sijson_new_value(SIJSON_STRING);
        if (value == NULL) {
            free(string);
            return NULL;
        }
        value->as.string = string;
        return value;
    }
    case '[':
        return sijson_parse_array_value(parser);
    case '{':
        return sijson_parse_object_value(parser);
    default:
        if (*parser->cur == '-' || isdigit((unsigned char)*parser->cur)) {
            return sijson_parse_number_value(parser);
        }
        sijson_set_error_at("expected JSON value", parser->cur);
        return NULL;
    }
}

sijson_value_t sijson_parse(const char *json) {
    sijson_clear_error();
    if (json == NULL) {
        sijson_set_error("sijson_parse expects JSON text");
        return NULL;
    }

    sijson_parser_t parser = { .cur = json };
    sijson_value_t value = sijson_parse_value(&parser);
    if (value == NULL) {
        return NULL;
    }

    sijson_skip_ws(&parser);
    if (*parser.cur != '\0') {
        sijson_set_error_at("trailing characters after JSON value", parser.cur);
        return NULL;
    }

    return value;
}
