#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "../gps/gps.h"
#include "../modem/modem.h"
#include "../sd/sd_card.h"

class WebServerManager {
public:
    WebServerManager(
        GPS& gps,
        Modem& modem,
        SDCard& sd
    );

    void begin();
    void update();

private:
    WebServer server;

    GPS& gps;
    Modem& modem;
    SDCard& sd;

    void handleRoot();
    void handleGPS();
    void handleAT();

    void handleTrackerStart();
    void handleTrackerStop();
    void handleDownload();

    String gpsHTML();
};
