#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <stdbool.h>
#include "esp_err.h"

// Initialize the management system (schedule, admin login, serial console)
esp_err_t management_init(void);

// Start the serial command interface (esp_console REPL on UART0)
esp_err_t management_start_cli(void);

// Check whether the building is currently open
bool management_is_open_now(void);

// Check whether normal building access is allowed
bool management_access_allowed(void);

// Check whether emergency exit override is enabled
bool management_emergency_exit_allowed(void);

// Update the building's opening and closing hours (admin only)
esp_err_t management_set_schedule(
    int open_hour,
    int open_minute,
    int close_hour,
    int close_minute
);

// Authenticate an administrator using a PIN
bool management_admin_login(const char *pin);

// Log out the administrator
void management_admin_logout(void);

// Check whether an administrator is logged in
bool management_admin_is_logged_in(void);

// Refresh the administrator session after activity
void management_admin_update_activity(void);

// Enable or disable emergency override (admin only)
esp_err_t management_set_emergency_override(bool enable);

#endif
