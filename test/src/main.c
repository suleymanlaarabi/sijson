
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
    }
};


static bake_test_suite suites[] = {
    {
        "simple",
        NULL,
        NULL,
        10,
        simple_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("sijson.test", argc, argv, suites, 1);
}
