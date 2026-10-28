#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// The SD card's FAT filesystem will be available at this path.
#define MOUNT_POINT "/sdcard"

// SPI pins connected to the SD card module.
// Keep these in sync with the wires in your Wokwi diagram.
#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   4

static const char *TAG = "BENJAMIN_STORAGE";

// The SD card details are filled in when the card is mounted.
static sdmmc_card_t *card;

/**
 * Set up the SPI bus, connect the SD card, and mount its FAT filesystem.
 * After a successful mount, files can be opened under MOUNT_POINT.
 */
static esp_err_t initialize_sd_card(void)
{
    esp_err_t ret;

    // Get the default SPI settings for an SD card connected over SPI.
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    // Use the same SPI host for the bus and the SD card device.
    spi_host_device_t spi_host = (spi_host_device_t)host.slot;

    // Tell ESP-IDF which GPIO pins carry the SPI signals.
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,  // Not used for ordinary SPI.
        .quadhd_io_num = -1,  // Not used for ordinary SPI.
        .max_transfer_sz = 4000,
    };

    ESP_LOGI(TAG, "Initializing SPI bus...");

    // Start the SPI bus before attaching the SD card to it.
    ret = spi_bus_initialize(spi_host, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus initialization failed: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    // Configure the SD card's chip-select pin and SPI host.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = spi_host;

    // Configure how the FAT filesystem should be mounted.
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,  // Do not erase/format on mount failure.
        .max_files = 5,                   // Maximum number of open files.
        .allocation_unit_size = 16 * 1024,
    };

    ESP_LOGI(TAG, "Mounting SD card...");

    // Initialize the card and make its FAT filesystem available at /sdcard.
    ret = esp_vfs_fat_sdspi_mount(
        MOUNT_POINT,
        &host,
        &slot_config,
        &mount_config,
        &card
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s",
                 esp_err_to_name(ret));

        // Release the SPI bus because mounting did not complete.
        spi_bus_free(spi_host);
        return ret;
    }

    ESP_LOGI(TAG, "SD card mounted at %s", MOUNT_POINT);
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

/**
 * Add one access event to the end of the SD card log.
 * The log uses one comma-separated record per line:
 * user ID, event
 */
static esp_err_t append_access_log(const char *user_id, const char *event)
{
    const char *file_path = MOUNT_POINT "/access_log.csv";

    // Append mode keeps earlier records and creates the file if needed.
    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        ESP_LOGE(TAG, "Could not open access log for writing");
        return ESP_FAIL;
    }

    // Write one record, such as: USER001,ENTRY
    if (fprintf(file, "%s,%s\n", user_id, event) < 0) {
        ESP_LOGE(TAG, "Could not write access record");
        fclose(file);
        return ESP_FAIL;
    }

    if (fclose(file) != 0) {
        ESP_LOGE(TAG, "Could not close access log");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Added access record for %s", user_id);
    return ESP_OK;
}

/**
 * Read the saved access log and print each record to the serial monitor.
 */
static esp_err_t display_access_log(void)
{
    const char *file_path = MOUNT_POINT "/access_log.csv";

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Could not open access log for reading");
        return ESP_FAIL;
    }

    char line[96];

    ESP_LOGI(TAG, "Saved access records:");

    while (fgets(line, sizeof(line), file) != NULL) {
        // fgets keeps the newline; remove it for a cleaner serial message.
        line[strcspn(line, "\r\n")] = '\0';
        ESP_LOGI(TAG, "  %s", line);
    }

    if (ferror(file)) {
        ESP_LOGE(TAG, "Error while reading access log");
        fclose(file);
        return ESP_FAIL;
    }

    fclose(file);
    return ESP_OK;
}

void app_main(void)
{
    // Stop with an error if the card cannot be initialized and mounted.
    ESP_ERROR_CHECK(initialize_sd_card());

    // Add one sample record, then print all records saved on the card.
    ESP_ERROR_CHECK(append_access_log("USER001", "ENTRY"));
    ESP_ERROR_CHECK(append_access_log("USER001", "EXIT"));
    ESP_ERROR_CHECK(display_access_log());

    // Keep the application running after the test.
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}