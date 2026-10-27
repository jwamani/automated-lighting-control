idf_component_register(
    SRCS "room_main.c" "light.c" "../../../main/net.c"
    INCLUDE_DIRS "." "../../../main"
    PRIV_REQUIRES esp_wifi esp_event esp_netif nvs_flash esp_hw_support esp_driver_gpio esp_timer esp_rom
)