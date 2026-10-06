#pragma once

#include <Arduino.h>

#define TINY_GSM_MODEM_A7670
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>

class Modem {
public:
    Modem();

    void begin();
    void update();

    bool test();

    String command(
        const String& command,
        uint32_t timeout = 3000
    );

private:
    TinyGsm modem;
};
