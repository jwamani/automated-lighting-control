#include <stdio.h>
#include "management.h"
#include "esp_log.h"
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

// Tracks whether an administrator is authenticated
static bool admin_logged_in = false;

// Temporary demonstration PIN
static const char *admin_pin = "2468";

// Maximum number of consecutive incorrect PIN attempts
#define MAX_LOGIN_ATTEMPTS 3

// Lockout duration in microseconds (60 seconds)
#define LOCKOUT_DURATION_US (60LL * 1000000LL)

// Number of consecutive failed login attempts
static int failed_login_attempts = 0;

// Time when the current lockout expires
static int64_t lockout_until = 0;




// Initialize the management system
esp_err_t management_init(void)
{
    if (initialized) {
        return ESP_OK;
    }

  emergency_override = false;
admin_logged_in = false;

failed_login_attempts = 0;
lockout_until = 0;
last_admin_activity = 0;
// Administrator session timeout: 5 minutes
#define SESSION_TIMEOUT_US (5LL * 60LL * 1000000LL)

static int64_t last_admin_activity = 0;

// Time of the administrator's last activity
static int64_t last_admin_activity = 0;



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
    // Only an authenticated administrator can update the schedule
if (!management_admin_is_logged_in()) {
    ESP_LOGW(TAG, "Access denied. Administrator login required.");
    return ESP_ERR_INVALID_STATE;
}

    // Check that hours and minutes are valid
    if (open_hour < 0 || open_hour > 23 ||
        close_hour < 0 || close_hour > 23 ||
        open_minute < 0 || open_minute > 59 ||
        close_minute < 0 || close_minute > 59) {

        ESP_LOGE(TAG, "Invalid time entered");
        management_admin_update_activity();
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

// Authenticate an administrator
// Authenticate an administrator with login attempt protection
bool management_admin_login(const char *pin)
{
    if (!initialized || pin == NULL) {
        return false;
    }

    // Get the current time in microseconds
    int64_t current_time = esp_timer_get_time();

    // Check whether the system is currently locked
    if (current_time < lockout_until) {
        ESP_LOGW(TAG, "Login temporarily locked. Try again later.");
        return false;
    }

    // Check whether the PIN is correct
   if (strcmp(pin, admin_pin) == 0) {
    admin_logged_in = true;

    failed_login_attempts = 0;
    lockout_until = 0;

    // Record when the administrator logged in
    last_admin_activity = current_time;

    ESP_LOGI(TAG, "Administrator login successful");

    return true;
}


    // Incorrect PIN
    failed_login_attempts++;

    ESP_LOGW(TAG, "Incorrect PIN. Failed attempts: %d",
             failed_login_attempts);

    // Lock the system after three incorrect attempts
    if (failed_login_attempts >= MAX_LOGIN_ATTEMPTS) {
        lockout_until = current_time + LOCKOUT_DURATION_US;

        failed_login_attempts = 0;

        ESP_LOGW(TAG, "Too many failed attempts. Login locked for 60 seconds.");
    }

    return false;
}

// End the administrator session
void management_admin_logout(void)
{
    admin_logged_in = false;
    last_admin_activity = 0;

    ESP_LOGI(TAG, "Administrator logged out");
}

// Check whether an administrator is logged in

// Check whether the administrator session is still valid
bool management_admin_is_logged_in(void)
{
    if (!initialized || !admin_logged_in) {
        return false;
    }

    int64_t current_time = esp_timer_get_time();

    // Check whether the session has expired
    if ((current_time - last_admin_activity) >= SESSION_TIMEOUT_US) {
        admin_logged_in = false;

        ESP_LOGW(TAG, "Administrator session expired");

        return false;
    }

    return true;
}

// Refresh the administrator's activity timestamp
void management_admin_update_activity(void)
{
    if (management_admin_is_logged_in()) {
        last_admin_activity = esp_timer_get_time();
    }
}

// Enable or disable emergency override
esp_err_t management_set_emergency_override(bool enable)
{
    // Only an authenticated administrator can change the override
    if (!management_admin_is_logged_in()) {
        ESP_LOGW(TAG, "Access denied. Administrator login required.");
        return ESP_ERR_INVALID_STATE;
    }

    // Update emergency override state
    emergency_override = enable;

    if (enable) {
        ESP_LOGW(TAG, "EMERGENCY OVERRIDE ACTIVATED");
    } else {
        ESP_LOGI(TAG, "Emergency override deactivated");
    }

    // Record administrator activity
    management_admin_update_activity();

    return ESP_OK;
}

// Display available CLI commands
static void management_print_help(void)
{
    printf("\n");
    printf("====================================\n");
    printf("   CoCIS BUILDING MANAGEMENT SYSTEM\n");
    printf("====================================\n");

    printf("Available commands:\n");

    printf("help                  - Show available commands\n");
    printf("status                - Show building status\n");
    printf("login                 - Administrator login\n");
    printf("logout                - Administrator logout\n");
    printf("schedule show         - Display building schedule\n");
    printf("schedule set          - Change building schedule\n");
    printf("override on           - Enable emergency override\n");
    printf("override off          - Disable emergency override\n");

    printf("====================================\n");
}

// Task responsible for reading administrator commands
static void management_cli_task(void *pvParameters)
{
    char command[100];

    printf("\nCoCIS Management CLI Started\n");
    printf("Type 'help' to view available commands.\n");

    while (1) {

        printf("\nCoCIS> ");
        fflush(stdout);

        // Read a command from the Serial Monitor
        if (fgets(command, sizeof(command), stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Remove the newline character
        command[strcspn(command, "\r\n")] = '\0';

        // Process the help command
        if (strcmp(command, "help") == 0) {
            management_print_help();
        }

        // Process the exit command
        else if (strcmp(command, "exit") == 0) {
            printf("CLI session ended.\n");
            break;
        }

        // Handle unknown commands
        else {
            printf("Unknown command. Type 'help' for assistance.\n");
        }
    }

    vTaskDelete(NULL);
}

// Start the management CLI FreeRTOS task
esp_err_t management_start_cli(void)
{
    if (!initialized) {
        ESP_LOGE(TAG, "Management system not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreate(
        management_cli_task,
        "management_cli",
        4096,
        NULL,
        5,
        NULL
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create management CLI task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Management CLI task created");

    return ESP_OK;
}

