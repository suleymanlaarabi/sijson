#include "sijson.h"
#include <sijson_test.h>

#include <stdlib.h>

SIJSON(User, {
    char *name;
    int age;
    bool active;
});

SIJSON(Profile, {
    char *name;
    char initial;
    u8 level;
    i16 score;
    u32 visits;
    f32 ratio;
    f64 weight;
    bool enabled;
});

SIJSON(Event, {
    char *name;
    sijson_value_t payload;
});

SIREFLECT_ENUM(Status, {
    STATUS_PENDING,
    STATUS_ACTIVE = 5,
    STATUS_DISABLED,
});

SIJSON(Account, {
    Status status;
    Status history[2];
});

void typed_to_json(void) {
    sireflect_init();
    User user = {
        .name = "Ada",
        .age = 37,
        .active = true,
    };

    char *json = sijson_to_json_ptr(User, &user);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");
    free(json);

    json = sijson_to_json(User, { .name = "Bob", .age = 41, .active = false });
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"Bob\",\"age\":41,\"active\":false}");
    free(json);

    user.name = NULL;
    json = sijson_to_json_ptr(User, &user);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":null,\"age\":37,\"active\":true}");
    free(json);
    sireflect_fini();
}

void typed_from_json(void) {
    sireflect_init();
    User user = sijson_from_json(User, "{\"name\":\"Ada\",\"age\":37,\"active\":true}");
    test_null((void *)sijson_error());
    test_str(user.name, "Ada");
    test_int(user.age, 37);
    test_true(user.active);
    sijson_free(User, &user);
    test_null(user.name);

    user = sijson_from_json(User, "{\"name\":null,\"age\":0,\"active\":false}");
    test_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);
    test_false(user.active);

    Profile profile = sijson_from_json(
        Profile,
        "{"
        "\"name\":\"Ada\","
        "\"initial\":\"A\","
        "\"level\":7,"
        "\"score\":-12,"
        "\"visits\":42,"
        "\"ratio\":1.5,"
        "\"weight\":2.25,"
        "\"enabled\":true"
        "}"
    );
    test_null((void *)sijson_error());
    test_str(profile.name, "Ada");
    test_int(profile.initial, 'A');
    test_uint(profile.level, 7);
    test_int(profile.score, -12);
    test_uint(profile.visits, 42);
    test_flt(profile.ratio, 1.5);
    test_flt(profile.weight, 2.25);
    test_true(profile.enabled);
    sijson_free(Profile, &profile);
    test_null(profile.name);
    sireflect_fini();
}

void typed_errors(void) {
    sireflect_init();
    User user = sijson_from_json(User, "{\"name\":\"Ada\",\"ignored\":123}");
    test_null((void *)sijson_error());
    test_str(user.name, "Ada");
    test_int(user.age, 0);
    test_false(user.active);
    sijson_free(User, &user);

    user = sijson_from_json(User, "{\"name\":42}");
    test_not_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);

    user = sijson_from_json(User, "{\"age\":1.5}");
    test_not_null((void *)sijson_error());
    test_null(user.name);
    test_int(user.age, 0);

    char *json = sijson_to_json_ptr(User, NULL);
    test_null(json);
    test_not_null((void *)sijson_error());
    sireflect_fini();
}

void typed_dynamic_value(void) {
    sireflect_init();
    Event event =
        sijson_from_json(Event, "{\"name\":\"created\",\"payload\":{\"kind\":\"user\",\"id\":12}}");
    test_null((void *)sijson_error());
    test_str(event.name, "created");
    test_uint(sijson_type(event.payload), SIJSON_OBJECT);
    test_str(sijson_string(sijson_object_get(event.payload, "kind")), "user");
    test_flt(sijson_number(sijson_object_get(event.payload, "id")), 12.0);

    char *json = sijson_to_json_ptr(Event, &event);
    test_null((void *)sijson_error());
    test_str(json, "{\"name\":\"created\",\"payload\":{\"kind\":\"user\",\"id\":12}}");
    free(json);

    sijson_free(Event, &event);
    test_null(event.name);
    sireflect_fini();
}

void typed_enum_fields(void) {
    sireflect_init();
    sireflect(Status);

    Account account = {
        .status = STATUS_ACTIVE,
        .history = { STATUS_PENDING, STATUS_DISABLED },
    };
    char *json = sijson_to_json_ptr(Account, &account);
    test_null((void *)sijson_error());
    test_str(json, "{\"status\":\"STATUS_ACTIVE\",\"history\":[\"STATUS_PENDING\",\"STATUS_DISABLED\"]}");

    Account parsed = sijson_from_json(
        Account,
        "{\"status\":\"STATUS_DISABLED\",\"history\":[\"STATUS_ACTIVE\",\"STATUS_PENDING\"]}"
    );
    test_null((void *)sijson_error());
    test_int(parsed.status, STATUS_DISABLED);
    test_int(parsed.history[0], STATUS_ACTIVE);
    test_int(parsed.history[1], STATUS_PENDING);
    free(json);

    parsed = sijson_from_json(Account, "{\"status\":\"STATUS_UNKNOWN\"}");
    test_not_null((void *)sijson_error());
    test_int(parsed.status, 0);
    sireflect_fini();
}
