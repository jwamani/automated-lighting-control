#include "management_data.h"

// Retrieve recent access records
esp_err_t management_get_access_records(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Clear the output buffer
    buffer[0] = '\0';

    // TODO: Connect to the access-record storage system
    return ESP_ERR_NOT_SUPPORTED;
}

// Retrieve the number of people currently inside the building
esp_err_t management_get_building_occupancy(int *count)
{
    if (count == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // No live occupancy data is connected yet
    *count = 0;

    // TODO: Connect to Esther's occupancy data
    return ESP_ERR_NOT_SUPPORTED;
}

// Retrieve the occupancy status of a particular room
esp_err_t management_get_room_occupancy(int room_id, bool *occupied)
{
    if (room_id <= 0 || occupied == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // No live room occupancy data is connected yet
    *occupied = false;

    // TODO: Connect to Sultana's room occupancy data
    return ESP_ERR_NOT_SUPPORTED;
}

// Retrieve the lighting status of a particular room
esp_err_t management_get_lighting_status(int room_id, bool *light_on)
{
    if (room_id <= 0 || light_on == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // No live lighting data is connected yet
    *light_on = false;

    // TODO: Connect to Sultana's lighting data
    return ESP_ERR_NOT_SUPPORTED;
}