idf_component_register(
     SRCS "member_b.c" "main.c" "access_ctrl.c" "rfid_reader.c" "door_ctrl.c"
         "lcd_i2c.c" "occupancy.c" "stubs.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_spi esp_driver_i2c
             esp_driver_ledc esp_timer
)