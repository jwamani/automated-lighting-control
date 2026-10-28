#pragma once

#include "esp_err.h"

/* Call after the SD card is mounted and before access_init(). */
esp_err_t member_b_init(void);
