// light.c - room lighting: PIR + ultrasonic presence, vacancy timeout, warning blink
// Owner: rooms
#include "light.h"
#include "room_config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "light";

static volatile bool building_open = true;
static volatile bool light_on = false;
static volatile bool occupied = false;

void light_set_building_open(bool open) { building_open = open; }
bool light_is_on(void)       { return light_on; }
bool light_is_occupied(void) { return occupied; }

static void set_light(bool on)
{
    if (on != light_on) ESP_LOGI(TAG, "light %s", on ? "ON" : "OFF");
    light_on = on;
    gpio_set_level(PIN_LIGHT, on);
}

static bool is_dark(void) { return gpio_get_level(PIN_LDR) == LDR_DARK_LEVEL; }

// HC-SR04: distance in cm, or -1 if no echo
static int read_distance_cm(void)
{
    gpio_set_level(PIN_TRIG, 0);
    esp_rom_delay_us(2);
    gpio_set_level(PIN_TRIG, 1);
    esp_rom_delay_us(10);
    gpio_set_level(PIN_TRIG, 0);

    int64_t t0 = esp_timer_get_time();
    while (!gpio_get_level(PIN_ECHO))
        if (esp_timer_get_time() - t0 > 30000) return -1;

    int64_t start = esp_timer_get_time();
    while (gpio_get_level(PIN_ECHO))
        if (esp_timer_get_time() - start > 30000) return -1;

    return (int)((esp_timer_get_time() - start) / 58);
}

static void light_task(void *arg)
{
    int64_t last_presence = 0;
    bool prev_present = false;

    while (1) {
        bool motion = gpio_get_level(PIN_PIR);
        int d = read_distance_cm();
        bool object = (d > 0 && d < EMPTY_DIST_CM - OBJECT_MARGIN_CM);
        bool present = motion || object;
        int64_t now = esp_timer_get_time() / 1000;   // ms

        if (present != prev_present) {
            ESP_LOGI(TAG, "presence=%d (pir=%d, dist=%dcm)", present, motion, d);
            prev_present = present;
        }

        if (!building_open) {
            occupied = false;
            set_light(false);                         // building closed: force off
        } else {
            if (present) {
                last_presence = now;
                occupied = true;
                if (is_dark()) set_light(true);
            }
            if (light_on) {
                int64_t idle = now - last_presence;
                if (idle >= VACANCY_TIMEOUT_MS) {
                    occupied = false;
                    set_light(false);                                // timeout: off
                } else if (idle >= VACANCY_TIMEOUT_MS - WARN_MS) {
                    gpio_set_level(PIN_LIGHT, (now / 250) % 2);      // warning blink
                } else {
                    gpio_set_level(PIN_LIGHT, 1);                    // steady on
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void light_start(void)
{
    gpio_reset_pin(PIN_PIR);   gpio_set_direction(PIN_PIR, GPIO_MODE_INPUT);
    gpio_reset_pin(PIN_LDR);   gpio_set_direction(PIN_LDR, GPIO_MODE_INPUT);
    gpio_reset_pin(PIN_ECHO);  gpio_set_direction(PIN_ECHO, GPIO_MODE_INPUT);
    gpio_reset_pin(PIN_TRIG);  gpio_set_direction(PIN_TRIG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PIN_LIGHT); gpio_set_direction(PIN_LIGHT, GPIO_MODE_OUTPUT);
    xTaskCreate(light_task, "light", 3072, NULL, 5, NULL);
}