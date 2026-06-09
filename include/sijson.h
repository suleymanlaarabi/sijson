#ifndef SIJSON_H
#define SIJSON_H

/* This generated file contains includes for project dependencies. */
#include "sijson/bake_config.h"
#include "sireflect.h"

/*
 * sijson is a tiny JSON serialization layer built on top of sireflect.
 *
 * Public usage is intentionally macro based:
 *
 *     SIJSON(Position, {
 *         float x;
 *         float y;
 *     });
 *
 *     char *json = sijson_to_json(Position, { .x = 1, .y = 2 });
 *     Position pos = sijson_from_json(Position, json);
 *
 * Strings returned by sijson_to_json are owned by the caller.
 * Values returned by sijson_from_json are copied out of an internal temporary
 * buffer before the expression ends. Do not keep pointers returned by the
 * lower-level _w_init functions.
 */

/* Internal reflected type handle symbol for a user type. */
#define sijson_handle(type) __sijson__##type##__handle

/*
 * Declare a JSON-reflectable type in a header.
 *
 * Pair this with SIJSON_DEFINE(type) in exactly one source file.
 */
#define SIJSON_DECLARE(type, ...)                                                                  \
    SIREFLECT_STRUCT(type, __VA_ARGS__);                                                           \
    extern sireflect_handle_t sijson_handle(type);

/* Define storage for a type declared with SIJSON_DECLARE. */
#define SIJSON_DEFINE(type) sireflect_handle_t sijson_handle(type) = 0;

/*
 * Declare and define a JSON-reflectable type in the current translation unit.
 *
 * This is the simplest form and is ideal for examples, tests, and small
 * programs.
 */
#define SIJSON(type, ...)                                                                          \
    SIJSON_DECLARE(type, __VA_ARGS__);                                                             \
    SIJSON_DEFINE(type);

/*
 * Default reflection registry used by the convenience macros.
 *
 * The implementation initializes it lazily when a type is first used.
 */
sireflect_registry_t *sijson_default_registry(void);

/*
 * Serialize a value written as a compound initializer.
 *
 * Example:
 *
 *     char *json = sijson_to_json(Position, { .x = 1, .y = 2 });
 *
 *     Position pos = { .x = 1, .y = 2 };
 *     char *json2 = sijson_to_json_ptr(Position, &pos);
 */
#define sijson_to_json(type, ...)                                                                  \
    sijson_to_json_impl(&sijson_handle(type), &sireflect_desc(type), &(type)__VA_ARGS__);

#define sijson_to_json_ptr(type, ptr) \
    sijson_to_json_impl(&sijson_handle(type), &sireflect_desc(type), (ptr))

/*
 * Low-level serialization entry point.
 *
 * Registers desc on first use through ref, then serializes ptr.
 * Returns a newly allocated JSON string owned by the caller.
 */
char *sijson_to_json_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    const void *ptr
);

/*
 * Deserialize JSON into a value of name.
 *
 * Example:
 *
 *     Position pos = sijson_from_json(Position, json);
 *
 * The returned value is copied from an internal temporary buffer. A later
 * sijson_from_json call may reuse that buffer, but previously copied structs
 * remain valid as long as they do not contain pointers into sijson memory.
 */
#define sijson_from_json(type, json)                                                               \
    *((type *)sijson_from_json_impl(&sijson_handle(type), &sireflect_desc(type), json))

/*
 * Low-level deserialization entry point.
 *
 * Registers desc on first use through ref, grows the internal temporary buffer
 * if needed, writes the parsed value into it, and returns that buffer.
 */
void *sijson_from_json_impl(
    sireflect_handle_t *ref,
    const sireflect_struct_desc_t *desc,
    const char *json
);

/*
 * Returns the error message from the last failed operation.
 */
const char *sijson_error(void);

#endif
