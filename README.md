# EFS LTE Tracker

This project is an LTE tracker firmware package for the LILYGO TTGO T-A7670G board (based around ESP32), designed to track location data using a modem, GPS, and SD card. The tracker logs data to the SD card and serves it via a web server.

## Features

- **LTE Connectivity**: Uses an A7670G modem for cellular communication.
- **GPS Tracking**: Integrates with an L76K GNSS module for precise location data.
- **Data Logging**: Stores tracking data on a microSD card for later retrieval.
- **Web Server**: Provides a local access point to view and manage tracked data.

## Hardware Requirements

- **ESP32 Board**: Main microcontroller.
- **A7670G Modem**: For LTE connectivity.
- **L76K GNSS**: GPS module for location tracking.
- **MicroSD Card**: For data logging.

## Software Setup

The firmware is built using PlatformIO with the following dependencies:
- TinyGSM for modem communication
- TinyGPSPlus for GPS parsing
- SdFat for SD card operations

## Usage

1. **Power On**: Connect the tracker to a power source.
2. **Access Web Server**: Connect to the WiFi access point `EFS_LTE_Tracker` with password `warg-efs-2026` to access the web interface.
3. **View Data**: Tracked data can be viewed and downloaded from the web interface.

## Configuration

The tracker can be configured by modifying the `src/board_config.h` file. Key settings include:
- Modem and GPS pin assignments
- Tracker data directory and logging interval
- WiFi access point credentials
