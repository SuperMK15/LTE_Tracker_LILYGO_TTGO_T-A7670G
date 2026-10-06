#include "gps.h"

#include "../board_config.h"

HardwareSerial SerialGPS(2);

GPS* GPS::instance = nullptr;


// ============================================================
// PPS INTERRUPT
// ============================================================

void IRAM_ATTR GPS::ppsISR()
{
    if (instance != nullptr) {
        instance->pps++;
        instance->ppsTime = millis();
    }
}


// ============================================================
// BEGIN
// ============================================================

void GPS::begin()
{
    instance = this;

    // L76K wakeup
    pinMode(
        GPS_WAKE_PIN,
        OUTPUT
    );

    // HIGH = operating
    digitalWrite(
        GPS_WAKE_PIN,
        HIGH
    );

    // PPS
    pinMode(
        GPS_PPS_PIN,
        INPUT
    );

    attachInterrupt(
        digitalPinToInterrupt(GPS_PPS_PIN),
        ppsISR,
        RISING
    );

    // GNSS UART
    SerialGPS.begin(
        GPS_BAUDRATE,
        SERIAL_8N1,
        GPS_RX_PIN,
        GPS_TX_PIN
    );

    currentLine.reserve(128);
    lastLine.reserve(128);

    Serial.println(
        "GPS initialized."
    );

    Serial.printf(
        "GPS RX  = GPIO %d\n",
        GPS_RX_PIN
    );

    Serial.printf(
        "GPS TX  = GPIO %d\n",
        GPS_TX_PIN
    );

    Serial.printf(
        "GPS WAKE = GPIO %d\n",
        GPS_WAKE_PIN
    );

    Serial.printf(
        "GPS PPS = GPIO %d\n",
        GPS_PPS_PIN
    );
}


// ============================================================
// UPDATE
// ============================================================

void GPS::update()
{
    while (SerialGPS.available()) {

        char c = SerialGPS.read();

        bytes++;

        lastByte = millis();

        // Feed byte into TinyGPS++
        gps.encode(c);

        // Capture NMEA sentence
        if (c == '\n') {

            currentLine.trim();

            if (currentLine.length() > 0) {
                sentences++;
                lastLine = currentLine;
            }

            currentLine = "";
        }
        else if (c != '\r') {

            currentLine += c;

            // Protect against malformed input
            if (currentLine.length() > 200) {
                currentLine = "";
            }
        }
    }
}


// ============================================================
// DATA
// ============================================================

TinyGPSPlus& GPS::data()
{
    return gps;
}


// ============================================================
// STATUS
// ============================================================

bool GPS::hasData() const
{
    return bytes > 0 &&
           (millis() - lastByte < 5000);
}


bool GPS::hasFix() const
{
    return gps.location.isValid() &&
           gps.location.age() < 5000;
}


// ============================================================
// DIAGNOSTICS
// ============================================================

uint32_t GPS::bytesReceived() const
{
    return bytes;
}


uint32_t GPS::sentencesReceived() const
{
    return sentences;
}


uint32_t GPS::ppsCount() const
{
    return pps;
}


uint32_t GPS::lastByteTime() const
{
    return lastByte;
}


uint32_t GPS::lastPPSTime() const
{
    return ppsTime;
}


const String& GPS::lastSentence() const
{
    return lastLine;
}
