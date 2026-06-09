#include "sijson_internal.h"

static char g_error[256];

void sijson_clear_error(void) { g_error[0] = '\0'; }

bool sijson_set_error(const char *message) {
    if (message == NULL) {
        message = "unknown sijson error";
    }

    snprintf(g_error, sizeof(g_error), "%s", message);
    return false;
}

bool sijson_set_error_at(const char *message, const char *at) {
    if (at == NULL) {
        return sijson_set_error(message);
    }

    snprintf(g_error, sizeof(g_error), "%s near '%.24s'", message, at);
    return false;
}

const char *sijson_error(void) { return g_error[0] != '\0' ? g_error : NULL; }
