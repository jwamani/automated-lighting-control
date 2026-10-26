#include "management.h"
#include "esp_log.h"
#include <stdbool.h>
#include <time.h>

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

// Check whether the building is currently open
bool management_is_open_now(void)
{
    time_t now;
    struct tm time_info;

    time(&now);

    localtime_r(&now, &time_info);

    return is_within_schedule(
        time_info.tm_hour,
        time_info.tm_min
    );
}

// Determine whether normal building access is allowed
bool management_access_allowed(void)
{
    if (!initialized) {
        return false;
    }

    return management_is_open_now();
}
 
// Check whether emergency exit override is enabled
bool management_emergency_exit_allowed(void)
{
    if (!initialized) {
        return false;
    }

    return emergency_override;
}

// Update the building's operating hours
esp_err_t management_set_schedule(
    int open_hour,
    int open_minute,
    int close_hour,
    int close_minute)
{
    // Check that hours and minutes are valid
    if (open_hour < 0 || open_hour > 23 ||
        close_hour < 0 || close_hour > 23 ||
        open_minute < 0 || open_minute > 59 ||
        close_minute < 0 || close_minute > 59) {

        ESP_LOGE(TAG, "Invalid time entered");
        return ESP_ERR_INVALID_ARG;
    }

    // Convert both times into minutes
    int opening = open_hour * 60 + open_minute;
    int closing = close_hour * 60 + close_minute;

    // Closing time must be later than opening time
    if (opening >= closing) {
        ESP_LOGE(TAG, "Closing time must be after opening time");
        return ESP_ERR_INVALID_ARG;
    }

    // Update the schedule
    opening_hour = open_hour;
    opening_minute = open_minute;

    closing_hour = close_hour;
    closing_minute = close_minute;

    ESP_LOGI(TAG, "Building schedule updated");

    ESP_LOGI(TAG, "New opening time: %02d:%02d",
             opening_hour, opening_minute);

    ESP_LOGI(TAG, "New closing time: %02d:%02d",
             closing_hour, closing_minute);

    return ESP_OK;
}