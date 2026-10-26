// room_config.h - pins and timings for the room terminal
#pragma once

#define PIN_PIR              4
#define PIN_LIGHT            5
#define PIN_LDR              6
#define PIN_TRIG             7
#define PIN_ECHO             15

#define EMPTY_DIST_CM        300
#define OBJECT_MARGIN_CM     50
#define VACANCY_TIMEOUT_MS   15000   // demo value; 600000 (10 min) in real life
#define WARN_MS              5000
#define LDR_DARK_LEVEL       1       // flip to 0 if the LDR behaves backwards