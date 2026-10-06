#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>

class GPS {
public:
    void begin();
    void update();

    TinyGPSPlus& data();

    bool hasData() const;
    bool hasFix() const;

    uint32_t bytesReceived() const;
    uint32_t sentencesReceived() const;
    uint32_t ppsCount() const;

    uint32_t lastByteTime() const;
    uint32_t lastPPSTime() const;

    const String& lastSentence() const;

private:
    TinyGPSPlus gps;

    uint32_t bytes = 0;
    uint32_t sentences = 0;

    uint32_t lastByte = 0;

    volatile uint32_t pps = 0;
    volatile uint32_t ppsTime = 0;

    String currentLine;
    String lastLine;

    static GPS* instance;

    static void IRAM_ATTR ppsISR();
};
