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

#endif