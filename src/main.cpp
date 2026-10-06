#include <Arduino.h>

#include "gps/gps.h"
#include "modem/modem.h"
#include "sd/sd_card.h"
#include "web/web_server.h"

GPS gps;
Modem modem;
SDCard sd;

WebServerManager web(
    gps,
    modem,
    sd
);

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("EFS LTE Tracker");
    Serial.println("================================");

    modem.begin();

    gps.begin();

    sd.begin();

    web.begin();

    Serial.println();
    Serial.println("System ready.");
}

void loop()
{
    gps.update();

    sd.update(gps);

    modem.update();

    web.update();

    delay(1);
}
