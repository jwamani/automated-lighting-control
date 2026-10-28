#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "app_config.h"
#include "net.h"
#include "management.h"

static const char *TAG = "MAIN_HUB";

void on_message(const char *topic, const char *data)
{
    ESP_LOGI(TAG, "RX %s: %s", topic, data);
}

void app_main(void)
{
    // Initialize the management system
    ESP_ERROR_CHECK(management_init());

    // Start the management CLI as a FreeRTOS task
    ESP_ERROR_CHECK(management_start_cli());

    // Start Wamani's MQTT hub
    net_start(HUB_CLIENT_ID, TOPIC_ROOMS_ALL, NULL);

    // Main hub loop
    while (1)
    {
        // Check whether the system clock is synchronized
        if (net_time_ok())
        {
            time_t now = time(NULL);
            struct tm t;

            localtime_r(&now, &t);

            ESP_LOGI(TAG, "Current time: %02d:%02d:%02d",
                     t.tm_hour, t.tm_min, t.tm_sec);
        }

        // Wait before checking again
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_PERIOD_MS));
    }
}