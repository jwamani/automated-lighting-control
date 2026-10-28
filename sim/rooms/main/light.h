// light.h - what light.c offers to the rest of the program (owner: rooms)
#pragma once
#include <stdbool.h>

void light_start(void);                    // call once, at the start of app_main
void light_set_building_open(bool open);   // hub says OPEN (true) or CLOSED (false)
bool light_is_on(void);
bool light_is_occupied(void);