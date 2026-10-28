#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "hub_types.h"
#include "member_b.h"

#define LOG_FILE_PATH "/sdcard/access_log.csv"
#define LOG_QUEUE_LENGTH 32

static const char *TAG = "member_b";

/* Demonstration users: replace these with the team's agreed user records. */
static const user_t s_users[] = {
    { "01020304", "Alice", "admin",   true  },
    { "11223344", "Bob",   "staff",   true  },
    { "55667788", "Carol", "student", true  },
    { "C0FFEE99", "Dave",  "student", false },
};

typedef struct {
    door_side_t side;
    char uid[UID_STR_LEN];
    char name[16];
    char result[24];
    int inside_count;
    int64_t uptime_ms;
} log_event_t;

static QueueHandle_t s_log_queue;

bool user_lookup(const char *uid, user_t *out)
{
    if (uid == NULL || out == NULL) {
        return false;
    }

    for (size_t i = 0; i < sizeof(s_users) / sizeof(s_users[0]); i++) {
        if (strcmp(s_users[i].uid, uid) == 0) {
            *out = s_users[i];
            return true;
        }
    }
    return false;
}

static const char *result_name(access_result_t result)
{
    switch (result) {
    case ACCESS_GRANTED:        return "GRANTED";
    case ACCESS_EMERGENCY_EXIT: return "EMERGENCY_EXIT";
    case DENIED_UNKNOWN:        return "DENIED_UNKNOWN";
    case DENIED_DISABLED:       return "DENIED_DISABLED";
    case DENIED_OUTSIDE_HOURS:  return "DENIED_OUTSIDE_HOURS";
    case DENIED_ALREADY_INSIDE: return "DENIED_ALREADY_INSIDE";
    case DENIED_NOT_INSIDE:     return "DENIED_NOT_INSIDE";
    default:                    return "ERROR";
    }
}

/* Quote CSV fields and escape any quote characters inside them. */
static void write_csv_field(FILE *file, const char *value)
{
    fputc('"', file);
    for (const char *p = value; *p != '\0'; p++) {
        if (*p == '"') {
            fputc('"', file);
        }
        fputc(*p, file);
    }
    fputc('"', file);
}

static void write_event(const log_event_t *event)
{
    FILE *file = fopen(LOG_FILE_PATH, "a+");
    if (file == NULL) {
        ESP_LOGE(TAG, "Cannot open %s; is the SD card mounted?",
                 LOG_FILE_PATH);
        return;
    }

    bool ok = true;

    if (fseek(file, 0, SEEK_END) != 0) {
        ok = false;
    } else if (ftell(file) == 0) {
        ok = fprintf(file,
                     "uptime_ms,side,uid,name,result,inside_count\n") >= 0;
    }

    if (ok) {
        ok = fprintf(file, "%lld,%s,",
                     (long long)event->uptime_ms,
                     event->side == SIDE_ENTRY ? "ENTRY" : "EXIT") >= 0;
    }

    if (ok) {
        write_csv_field(file, event->uid);
        fputc(',', file);
        write_csv_field(file, event->name);
        fputc(',', file);
        write_csv_field(file, event->result);
        ok = fprintf(file, ",%d\n", event->inside_count) >= 0;
    }

    if (fclose(file) != 0) {
        ok = false;
    }

    if (!ok) {
        ESP_LOGE(TAG, "Failed writing access event to SD card");
    }
}

/* This is the single writer for the log file, so its writes are serialized. */
static void log_task(void *arg)
{
    log_event_t event;

    while (true) {
        if (xQueueReceive(s_log_queue, &event, portMAX_DELAY) == pdTRUE) {
            write_event(&event);
        }
    }
}

esp_err_t member_b_init(void)
{
    if (s_log_queue != NULL) {
        return ESP_OK;
    }

    s_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_event_t));
    if (s_log_queue == NULL) {
        return ESP_ERR_NO_MEM;
    }

    if (xTaskCreate(log_task, "access_log_task", 4096, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(s_log_queue);
        s_log_queue = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "User table and SD logger ready");
    return ESP_OK;
}

/* Called by the access-control module after each decision. */
void log_access(door_side_t side, const char *uid, const char *name,
                access_result_t result, int inside_count)
{
    if (s_log_queue == NULL || uid == NULL || name == NULL) {
        ESP_LOGW(TAG, "Logger not ready or event is incomplete; event skipped");
        return;
    }

    log_event_t event = {
        .side = side,
        .inside_count = inside_count,
        .uptime_ms = esp_timer_get_time() / 1000
    };

    strlcpy(event.uid, uid, sizeof(event.uid));
    strlcpy(event.name, name, sizeof(event.name));
    strlcpy(event.result, result_name(result), sizeof(event.result));

    if (xQueueSend(s_log_queue, &event, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Log queue full; access event dropped");
    }
}
