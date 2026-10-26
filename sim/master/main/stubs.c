#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "hub_config.h"
#include "hub_interfaces.h"
#include "access_ctrl.h"

#if USE_STUBS

static const char *TAG = "stub";

static const user_t k_users[] = {
    { "01020304", "Alice", "admin",   true  },  /* Wokwi Blue card   */
    { "11223344", "Bob",   "staff",   true  },  /* Wokwi Green card  */
    { "55667788", "Carol", "student", true  },  /* Wokwi Yellow card */
    { "C0FFEE99", "Dave",  "student", false },  /* Wokwi Key Fob, switched off */
};
/* The Red card (AABBCCDD) and the NFC tag are NOT listed on purpose:
   they are the "unknown card" test cards. */

bool user_lookup(const char *uid, user_t *out)
{
    for (size_t i = 0; i < sizeof(k_users) / sizeof(k_users[0]); i++) {
        if (strcmp(k_users[i].uid, uid) == 0) {
            *out = k_users[i];
            return true;
        }
    }
    return false;
}

bool schedule_is_open(void)
{
    static int last = -1;
    int64_t sec = esp_timer_get_time() / 1000000;
    bool open = (sec % (STUB_OPEN_SECONDS + STUB_CLOSED_SECONDS)) < STUB_OPEN_SECONDS;
    if (last != (int)open) {
        last = (int)open;
        ESP_LOGW(TAG, "Building is now %s", open ? "OPEN" : "CLOSED");
    }
    return open;
}

bool override_active(void)
{
    return false;
}

void log_access(door_side_t side, const char *uid, const char *name,
                access_result_t result, int inside_count)
{
    ESP_LOGI("log", "%s uid=%s name=%s -> %s (inside=%d)",
             side == SIDE_ENTRY ? "ENTRY" : "EXIT", uid, name,
             access_result_text(result), inside_count);
}

#endif /* USE_STUBS */