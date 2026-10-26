#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <stdbool.h>
#include "esp_err.h"

// Initialize the management system
esp_err_t management_init(void);

// Start the serial command interface
esp_err_t management_start_cli(void);

// Check whether the building is open
bool management_is_open_now(void);

// Check whether normal building access is allowed
bool management_access_allowed(void);

// Check whether emergency exit is enabled
bool management_emergency_exit_allowed(void);

// Update the building's opening and closing hours
esp_err_t management_set_schedule(
    int open_hour,
    int open_minute,
    int close_hour,
    int close_minute
);
// Authenticate an administrator using a PIN
bool management_admin_login(const char *pin);

// Log out the administrator
void management_admin_logout(void)
{
    admin_logged_in = false;
    last_admin_activity = 0;

    ESP_LOGI(TAG, "Administrator logged out");
}
// Check whether an administrator is logged in
bool management_admin_is_logged_in(void);

// Refresh the administrator session after activity
void management_admin_update_activity(void);

#endif