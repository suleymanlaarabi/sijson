
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <test.h>

// Testsuite 'simple'
void simple_dynamic_make(void);
void simple_dynamic_arena(void);
void simple_dynamic_parse(void);
void simple_stringify(void);
void simple_dynamic_errors(void);
void simple_registry(void);
void simple_typed_to_json(void);
void simple_typed_from_json(void);
void simple_typed_errors(void);
void simple_typed_dynamic_value(void);
void simple_typed_array_to_json_numbers(void);
void simple_typed_array_from_json_numbers(void);
void simple_typed_array_roundtrip_matrix(void);
void simple_typed_array_of_structs(void);
void simple_typed_array_of_strings(void);
void simple_typed_array_rejects_wrong_length(void);
void simple_typed_array_rejects_wrong_type(void);
void simple_typed_native_aliases_to_json(void);
void simple_typed_native_aliases_from_json(void);
void simple_typed_native_aliases_reject_range(void);

bake_test_case simple_testcases[] = {
    {
        "dynamic_make",
        simple_dynamic_make
    },
    {
        "dynamic_arena",
        simple_dynamic_arena
    },
    {
        "dynamic_parse",
        simple_dynamic_parse
    },
    {
        "stringify",
        simple_stringify
    },
    {
        "dynamic_errors",
        simple_dynamic_errors
    },
    {
        "registry",
        simple_registry
    },
    {
        "typed_to_json",
        simple_typed_to_json
    },
    {
        "typed_from_json",
        simple_typed_from_json
    },
    {
        "typed_errors",
        simple_typed_errors
    },
    {
        "typed_dynamic_value",
        simple_typed_dynamic_value
    },
    {
        "typed_array_to_json_numbers",
        simple_typed_array_to_json_numbers
    },
    {
        "typed_array_from_json_numbers",
        simple_typed_array_from_json_numbers
    },
    {
        "typed_array_roundtrip_matrix",
        simple_typed_array_roundtrip_matrix
    },
    {
        "typed_array_of_structs",
        simple_typed_array_of_structs
    },
    {
        "typed_array_of_strings",
        simple_typed_array_of_strings
    },
    {
        "typed_array_rejects_wrong_length",
        simple_typed_array_rejects_wrong_length
    },
    {
        "typed_array_rejects_wrong_type",
        simple_typed_array_rejects_wrong_type
    },
    {
        "typed_native_aliases_to_json",
        simple_typed_native_aliases_to_json
    },
    {
        "typed_native_aliases_from_json",
        simple_typed_native_aliases_from_json
    },
    {
        "typed_native_aliases_reject_range",
        simple_typed_native_aliases_reject_range
    }
};


static bake_test_suite suites[] = {
    {
        "simple",
        NULL,
        NULL,
        20,
        simple_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("sijson.test", argc, argv, suites, 1);
}
