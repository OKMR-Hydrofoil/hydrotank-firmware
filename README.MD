# Hydrotank Firmware

A lightweight PlatformIO firmware for the Hydrotank project — sensor-driven force measurement for wing simulation.

## Table of Contents

- Overview
- Features
- Hardware
- Pin Assignments
- Build & Flash
- Configuration
- Project Layout
- Usage
- Troubleshooting
- Contributing

## Overview

This firmware is designed to run on microcontroller boards (e.g., ESP32, Arduino) to monitor the force produced by a wing in a hydrotank setup. It reads the force from load cells, processes the data, and displays the results via an oled display.

## Features

- PlatformIO-based build system
- Sensor sampling and filtering
- Realtime force calculation and scaling
- OLED display support for live data visualization

## Hardware

- MCU / board: ESP32 DevKitV1
- Power supply: USB 5V
- Sensors: Load cells with HX711 amplifier, Rotary encoder
- Communication: SPI, I2C, UART (for debugging), WiFi (optional)

## Pin Assignments

Populate the following table with your board's pin mappings.

| Signal | Function       | Board Pin | Notes |
| ------ | -------------- | --------- | ----- |
| SCL    | I2C Clock OLED | D33       |       |
| SDA    | I2C Data OLED  | D32       |       |
| A      | Encoder A      | D25       |       |
| B      | Encoder B      | D26       |       |

To be updated when I fix bad pins as D35 and D34 are outputs only on ESP32 DevKitV1.

## Build & Flash

Prerequisites: PlatformIO (install via VS Code or pip).

Common commands:

```bash
# build
pio run

# build + upload to default env
pio run -t upload

# specify environment (match platformio.ini)
pio run -e <env> -t upload

# monitor serial output (adjust baud as needed)
pio device monitor -b 115200
```

## Configuration

- Edit platformio.ini to choose board/environment and build flags.
- Move hardware-specific pin defines into src/config.h or platformio build flags.
- Runtime settings (timers, thresholds) can be stored in EEPROM/FLASH — document where in code to change them.

## Project Layout

- src/ — firmware source
- include/ — headers and public configs
- lib/ — external libraries
- data/ — SPIFFS/LittleFS files (if applicable)
- platformio.ini — build configuration

## Usage

1. Configure pins and hardware constants in src/config.h (or appropriate file).
2. Build and flash using PlatformIO.
3. Open serial monitor to observe boot logs and sensor readings.
4. Test pump control and safety interlocks in a safe environment.

## Troubleshooting

- No serial output: check UART pins and baud rate; ensure board selected in platformio.ini.
- Sensors reading erratic: verify wiring, pull-up/pull-down resistors, and ADC reference.

## Contributing

- Fork, create feature branches, and open PRs.
- Keep changes modular and document hardware changes clearly.
