#include "sijson.h"
#include <sijson_test.h>

#include <stdlib.h>

SIJSON(NativeAliases, {
    signed char code;
    unsigned char byte;
    unsigned short small;
    unsigned int flags;
    unsigned long span;
    long long count;
    unsigned long long mask;
});

SIJSON(NativeAliasArrays, {
    unsigned long long values[2];
    signed char codes[3];
});

void typed_native_aliases_to_json(void) {
    sireflect_init();
    NativeAliases aliases = {
        .code = -12,
        .byte = 250,
        .small = 65000,
        .flags = 4000000000u,
        .span = 123456789ul,
        .count = -9000000000ll,
        .mask = 1800000000000000ull,
    };

    char *json = sijson_to_json_ptr(NativeAliases, &aliases);
    test_null((void *)sijson_error());
    test_str(
        json,
        "{\"code\":-12,\"byte\":250,\"small\":65000,\"flags\":4000000000,\"span\":123456789,"
        "\"count\":-9000000000,\"mask\":1800000000000000}"
    );
    free(json);

    NativeAliasArrays arrays = {
        .values = { 11ull, 22ull },
        .codes = { -1, 0, 1 },
    };
    json = sijson_to_json_ptr(NativeAliasArrays, &arrays);
    test_null((void *)sijson_error());
    test_str(json, "{\"values\":[11,22],\"codes\":[-1,0,1]}");
    free(json);
    sireflect_fini();
}

void typed_native_aliases_from_json(void) {
    sireflect_init();
    NativeAliases aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":-12,\"byte\":250,\"small\":65000,\"flags\":4000000000,\"span\":123456789,"
        "\"count\":-9000000000,\"mask\":1800000000000000}"
    );
    test_null((void *)sijson_error());
    test_int(aliases.code, -12);
    test_uint(aliases.byte, 250);
    test_uint(aliases.small, 65000);
    test_uint(aliases.flags, 4000000000u);
    test_uint(aliases.span, 123456789ul);
    test_int(aliases.count, -9000000000ll);
    test_uint(aliases.mask, 1800000000000000ull);

    NativeAliasArrays arrays =
        sijson_from_json(NativeAliasArrays, "{\"values\":[11,22],\"codes\":[-1,0,1]}");
    test_null((void *)sijson_error());
    test_uint(arrays.values[0], 11);
    test_uint(arrays.values[1], 22);
    test_int(arrays.codes[0], -1);
    test_int(arrays.codes[1], 0);
    test_int(arrays.codes[2], 1);
    sireflect_fini();
}

void typed_native_aliases_reject_range(void) {
    sireflect_init();
    NativeAliases aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":128,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_int(aliases.code, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":-1,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.byte, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":65536,\"flags\":0,\"span\":0,\"count\":0,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.small, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":1.5,\"mask\":0}"
    );
    test_not_null((void *)sijson_error());
    test_int(aliases.count, 0);

    aliases = sijson_from_json(
        NativeAliases,
        "{\"code\":0,\"byte\":0,\"small\":0,\"flags\":0,\"span\":0,\"count\":0,\"mask\":-1}"
    );
    test_not_null((void *)sijson_error());
    test_uint(aliases.mask, 0);
    sireflect_fini();
}
