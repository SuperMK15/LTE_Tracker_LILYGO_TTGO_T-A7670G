#pragma once

// ============================================================
// A7670G MODEM
// ============================================================

#define MODEM_BAUDRATE       115200

#define MODEM_DTR_PIN        25
#define MODEM_TX_PIN         26
#define MODEM_RX_PIN         27

#define MODEM_PWRKEY_PIN     4
#define MODEM_POWERON_PIN    12
#define MODEM_RESET_PIN      5
#define MODEM_RESET_LEVEL    HIGH


// ============================================================
// L76K GNSS
// ============================================================
//
// L76K TX -> ESP32 RX GPIO22
// L76K RX <- ESP32 TX GPIO21
// L76K PPS -> GPIO23
// L76K WAKE <- GPIO19
//
// ============================================================

#define GPS_BAUDRATE         9600

#define GPS_RX_PIN           22
#define GPS_TX_PIN           21
#define GPS_PPS_PIN          23
#define GPS_WAKE_PIN         19


// ============================================================
// MICROSD
// ============================================================

#define SD_SCK_PIN           14
#define SD_MISO_PIN          2
#define SD_MOSI_PIN          15
#define SD_CS_PIN            13

// GPIO12 powers the board peripherals, including SD/modem.
#define PERIPHERAL_POWER_PIN 12


// ============================================================
// TRACKER
// ============================================================

#define TRACKER_DIRECTORY    "/tracker_data"
#define TRACKER_INTERVAL_MS  1000


// ============================================================
// WIFI ACCESS POINT
// ============================================================

#define AP_SSID              "EFS_LTE_Tracker"
#define AP_PASSWORD          "warg-efs-2026"
