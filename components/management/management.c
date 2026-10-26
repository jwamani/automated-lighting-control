#include "management.h"
#include "esp_log.h"
#include <stdbool.h>

// Identifies messages from our management module
static const char *TAG = "MANAGEMENT";

// Default building opening and closing hours
static int opening_hour = 8;
static int opening_minute = 0;

static int closing_hour = 17;
static int closing_minute = 0;

// Emergency override starts disabled
static bool emergency_override = false;

// Tracks whether the module has been initialized
static bool initialized = false;

// Initialize the management system
esp_err_t management_init(void)
{
    if (initialized) {
        return ESP_OK;
    }

    emergency_override = false;

    initialized = true;

    ESP_LOGI(TAG, "Management system initialized");

    ESP_LOGI(TAG, "Opening time: %02d:%02d",
             opening_hour, opening_minute);

    ESP_LOGI(TAG, "Closing time: %02d:%02d",
             closing_hour, closing_minute);

    return ESP_OK;
}
// Check whether a given time falls within building opening hours
static bool is_within_schedule(int hour, int minute)
{
    int current_time = hour * 60 + minute;

    int opening_time = opening_hour * 60 + opening_minute;

    int closing_time = closing_hour * 60 + closing_minute;

    return (current_time >= opening_time &&
            current_time < closing_time);
}