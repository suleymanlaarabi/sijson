
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <test.h>

// Testsuite 'dynamic'
void dynamic_make(void);
void dynamic_arena(void);
void dynamic_parse(void);
void dynamic_stringify(void);
void dynamic_errors(void);
void dynamic_registry(void);

// Testsuite 'typed'
void typed_to_json(void);
void typed_from_json(void);
void typed_errors(void);
void typed_dynamic_value(void);

// Testsuite 'typed_array'
void typed_array_to_json_numbers(void);
void typed_array_from_json_numbers(void);
void typed_array_roundtrip_matrix(void);
void typed_array_of_structs(void);
void typed_array_of_strings(void);
void typed_array_rejects_wrong_length(void);
void typed_array_rejects_wrong_type(void);

// Testsuite 'typed_native_aliases'
void typed_native_aliases_to_json(void);
void typed_native_aliases_from_json(void);
void typed_native_aliases_reject_range(void);

bake_test_case dynamic_testcases[] = {
    {
        "make",
        dynamic_make
    },
    {
        "arena",
        dynamic_arena
    },
    {
        "parse",
        dynamic_parse
    },
    {
        "stringify",
        dynamic_stringify
    },
    {
        "errors",
        dynamic_errors
    },
    {
        "registry",
        dynamic_registry
    }
};

bake_test_case typed_testcases[] = {
    {
        "to_json",
        typed_to_json
    },
    {
        "from_json",
        typed_from_json
    },
    {
        "errors",
        typed_errors
    },
    {
        "dynamic_value",
        typed_dynamic_value
    }
};

bake_test_case typed_array_testcases[] = {
    {
        "to_json_numbers",
        typed_array_to_json_numbers
    },
    {
        "from_json_numbers",
        typed_array_from_json_numbers
    },
    {
        "roundtrip_matrix",
        typed_array_roundtrip_matrix
    },
    {
        "of_structs",
        typed_array_of_structs
    },
    {
        "of_strings",
        typed_array_of_strings
    },
    {
        "rejects_wrong_length",
        typed_array_rejects_wrong_length
    },
    {
        "rejects_wrong_type",
        typed_array_rejects_wrong_type
    }
};

bake_test_case typed_native_aliases_testcases[] = {
    {
        "to_json",
        typed_native_aliases_to_json
    },
    {
        "from_json",
        typed_native_aliases_from_json
    },
    {
        "reject_range",
        typed_native_aliases_reject_range
    }
};


static bake_test_suite suites[] = {
    {
        "dynamic",
        NULL,
        NULL,
        6,
        dynamic_testcases
    },
    {
        "typed",
        NULL,
        NULL,
        4,
        typed_testcases
    },
    {
        "typed_array",
        NULL,
        NULL,
        7,
        typed_array_testcases
    },
    {
        "typed_native_aliases",
        NULL,
        NULL,
        3,
        typed_native_aliases_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("sijson.test", argc, argv, suites, 4);
}
