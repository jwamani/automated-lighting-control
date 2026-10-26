#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_mac.h"

#include "app_config.h"
#include "net.h"

static const char *TAG = "room";
static char client_id[32];

void on_message(const char *topic, const char *data)
{
    ESP_LOGI(TAG, "building is %s", data);
}

void app_main(void)
{
    uint8_t mac[6];
    char status_topic[64];
    char senser_topic[64];

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    snprintf(client_id, sizeof(client_id), ROOM_CLIENT_PREFIX "%02x%02x%02x", mac[3], mac[4], mac[5]);
    snprintf(status_topic, sizeof(status_topic), "cocis-g1/rooms/%02x%02x%02x/online", mac[3], mac[4], mac[5]);
    snprintf(senser_topic, sizeof(senser_topic), "cocis-g1/rooms/%02x%02x%02x/status", mac[3], mac[4], mac[5]);

    net_start(client_id, TOPIC_HUB, status_topic);

    char msg[64];
    while (1)
    {
        snprintf(msg, sizeof(msg), "%s occupied=0 light=0", client_id);
        net_publish(senser_topic, msg);
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_PERIOD_MS));
    }
}