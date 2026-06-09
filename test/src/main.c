
/* A friendly warning from bake.test
 * ----------------------------------------------------------------------------
 * This file is generated. To add/remove testcases modify the 'project.json' of
 * the test project. ANY CHANGE TO THIS FILE IS LOST AFTER (RE)BUILDING!
 * ----------------------------------------------------------------------------
 */

#include <test.h>

// Testsuite 'simple'
void simple_hello(void);

bake_test_case simple_testcases[] = {
    {
        "hello",
        simple_hello
    }
};


static bake_test_suite suites[] = {
    {
        "simple",
        NULL,
        NULL,
        1,
        simple_testcases
    }
};

int main(int argc, char *argv[]) {
    return bake_test_run("sijson.test", argc, argv, suites, 1);
}
