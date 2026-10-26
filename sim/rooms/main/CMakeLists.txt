idf_component_register(
    SRCS "room_main.c" "light.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio esp_timer esp_rom
)