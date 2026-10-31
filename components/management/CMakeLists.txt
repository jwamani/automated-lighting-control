idf_component_register(
    SRCS
        "management.c"
        "management_data.c"
    INCLUDE_DIRS "."
    REQUIRES esp_timer
)