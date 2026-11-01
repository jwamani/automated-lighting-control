# Automated Lighting Control

An ESP32-S3-based building access and automated-lighting system simulated with
Wokwi. A central hub controls building access, while two room terminals detect
occupancy, control their lights, and report status to the hub over MQTT.

The system uses the Wokwi virtual Wi-Fi network (`Wokwi-GUEST`) and the public
MQTT broker `broker.emqx.io:1883`. The MQTT topic prefix is `cocis-g1/`.

## System Components

### Building hub

The root project and `sim/master` contain the hub firmware. The hub:

- connects to Wi-Fi and MQTT;
- receives room status, online, and intrusion messages;
- manages entry and exit access using RFID readers;
- tracks occupancy and prevents invalid entry or exit events;
- controls entry and exit doors with servos;
- displays access and building information on an I2C LCD;
- indicates access decisions with green and red LEDs;
- manages opening hours, administrator authentication, and emergency exit mode;
- stores users and access, room, and system logs on the simulated SD card;
- publishes the building state (`open`, `closed`, or `emergency`) to the rooms.

### Room terminals

`sim/room_1` and `sim/room_2` contain the room firmware. Each room terminal:

- detects occupancy using a PIR sensor and ultrasonic distance sensor;
- measures ambient light with an LDR;
- switches its lamp according to occupancy and light level;
- turns the lamp off after the configured vacancy timeout;
- reports occupancy, light, energy-saving, and intrusion status to the hub;
- responds to the building state published by the hub.

### Shared and project files

| Path                                 | Purpose                                            |
| ------------------------------------ | -------------------------------------------------- |
| `main/`                              | Main hub implementation and shared networking code |
| `main/app_config.h`                  | Wi-Fi, MQTT broker, topic, and timing settings     |
| `main/hub_config.h`                  | Hub GPIO assignments and access-control timings    |
| `sim/master/`                        | Standalone Wokwi hub workspace                     |
| `sim/room_1/`                        | Standalone Wokwi room-terminal workspace           |
| `sim/room_2/`                        | Second room-terminal workspace                     |
| `diagram.json`                       | Root hub circuit diagram                           |
| `mosquitto.conf`                     | Optional local Mosquitto broker configuration      |
| `sdkconfig` and `sdkconfig.defaults` | ESP-IDF project configuration                      |

## MQTT Communication

The hub subscribes to `cocis-g1/rooms/#`. Room terminals publish status and
presence information under topics such as:

```text
cocis-g1/rooms/<room-id>/status
cocis-g1/rooms/<room-id>/online
cocis-g1/rooms/alert
```

The hub publishes the current building state on:

```text
cocis-g1/building
```

The public broker is shared by other users, so do not publish passwords,
personal data, or other sensitive information.

## Requirements

- ESP-IDF 5.3 or newer. The current development setup uses ESP-IDF 6.0.2.
- ESP32-S3 toolchain and Python environment installed by ESP-IDF.
- VS Code with the Wokwi extension.
- PowerShell or the ESP-IDF PowerShell terminal. Do not use Git Bash, MSYS, or
  MinGW shells; current ESP-IDF tooling does not support them.
- `mosquitto_sub` is optional and is only needed to inspect MQTT traffic from a
  host machine.

## Setup and Build

Open the repository in VS Code, select the ESP-IDF installation, and open an
ESP-IDF PowerShell terminal in the relevant workspace. For a fresh project,
set the target once before building:

```powershell
idf.py set-target esp32s3
idf.py build
```

Build the workspaces independently:

```powershell
# Building hub sources from the repository root
cd C:\path\to\automated-lighting-control
idf.py set-target esp32s3
idf.py build

# Building room 1 and room 2
cd sim\room_1
idf.py set-target esp32s3
idf.py build

cd sim\room_2
idf.py set-target esp32s3
idf.py build
```

Each Wokwi workspace expects its own `build` directory. The corresponding
`wokwi.toml` file selects the generated ELF and flash arguments.

## Run the Simulation

1. Build the hub and both room workspaces as shown above.
2. Open `sim/master` in one VS Code window and run `Wokwi: Start Simulator`.
3. Open `sim/room_1` and `sim/room_2` in separate VS Code windows.
4. Run `Wokwi: Start Simulator` in both room workspaces.
5. Use each Wokwi serial monitor to view access decisions, room reports, and
   MQTT connection messages.

The hub diagram contains two MFRC522 RFID readers, two door servos, an I2C
LCD, entry and exit status LEDs, and a microSD card. The room diagrams contain
the sensors and lamp used for occupancy-based lighting control.

To inspect all project messages from a machine with Mosquitto installed:

```powershell
mosquitto_sub -h broker.emqx.io -t "cocis-g1/#" -v
```

Or if you find the public brokers unreliable, you can run a local mosquitto broker with the provided `mosquitto.conf` file and make sure to listen to all interfaces. You can then pass in your pc's IP address to the `app_config.h` under the `MQTT_BROKER` define. You can then run the same command as above to inspect all messages.
