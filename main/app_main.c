#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_config.h"
#include "net.h"
#include "rooms.h"
#include "building.h"
#include "access_ctrl.h"
#include "storage.h"
#include "management.h"

static const char *TAG = "MAIN_HUB";

void on_message(const char *topic, const char *data)
{
    rooms_handle_message(topic, data);
}

void app_main(void)
{
    // management component: schedule, admin login and the serial CLI (member E)
    ESP_ERROR_CHECK(management_init());
    ESP_ERROR_CHECK(management_start_cli());

    // network in the background; the hub keeps working without wifi or broker
    net_start(HUB_CLIENT_ID, TOPIC_ROOMS_ALL, NULL);
    rooms_init();
    storage_init();  // SD card mount + users.csv into RAM (before RFID starts)
    access_init();   // occupancy list, doors, decision task, RFID readers

    while (1) {
        building_announce();                       // retained: open/closed/emergency + inside count
        // rooms_tick();                           // optional safety net: flag rooms that stopped reporting
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_PERIOD_MS));
    }
}
