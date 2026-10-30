idf_component_register(
    SRCS "net.c" "app_main.c" "management_bridge.c" "rooms.c" "building.c"
         "access_ctrl.c" "occupancy.c" "door_ctrl.c" "lcd_i2c.c"
         "rfid_reader.c" "storage.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_wifi esp_event esp_netif nvs_flash management
                  esp_driver_gpio esp_driver_spi esp_driver_i2c
                  esp_driver_ledc esp_timer
                  fatfs sdmmc esp_driver_sdspi
)
