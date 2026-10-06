#include "modem.h"

#include "../board_config.h"

HardwareSerial SerialAT(1);


// ============================================================
// CONSTRUCTOR
// ============================================================

Modem::Modem()
    : modem(SerialAT)
{
}


// ============================================================
// BEGIN
// ============================================================

void Modem::begin()
{
    // Modem UART
    SerialAT.begin(
        MODEM_BAUDRATE,
        SERIAL_8N1,
        MODEM_RX_PIN,
        MODEM_TX_PIN
    );


    // ========================================================
    // POWER ENABLE
    // ========================================================

    pinMode(
        MODEM_POWERON_PIN,
        OUTPUT
    );

    digitalWrite(
        MODEM_POWERON_PIN,
        HIGH
    );


    // ========================================================
    // RESET
    // ========================================================

    pinMode(
        MODEM_RESET_PIN,
        OUTPUT
    );

    digitalWrite(
        MODEM_RESET_PIN,
        !MODEM_RESET_LEVEL
    );

    delay(100);

    digitalWrite(
        MODEM_RESET_PIN,
        MODEM_RESET_LEVEL
    );

    delay(2600);

    digitalWrite(
        MODEM_RESET_PIN,
        !MODEM_RESET_LEVEL
    );


    // ========================================================
    // POWER KEY
    // ========================================================

    pinMode(
        MODEM_PWRKEY_PIN,
        OUTPUT
    );

    digitalWrite(
        MODEM_PWRKEY_PIN,
        LOW
    );

    delay(100);

    digitalWrite(
        MODEM_PWRKEY_PIN,
        HIGH
    );

    delay(100);

    digitalWrite(
        MODEM_PWRKEY_PIN,
        LOW
    );


    // ========================================================
    // WAIT FOR MODEM
    // ========================================================

    Serial.print(
        "Waiting for modem"
    );

    while (!modem.testAT(1000)) {

        Serial.print(".");

        delay(1000);
    }

    Serial.println();

    Serial.println(
        "Modem is online."
    );


    // Disable command echo
    command("ATE0");

    // Enable verbose errors
    command("AT+CMEE=2");
}


// ============================================================
// TEST
// ============================================================

bool Modem::test()
{
    return modem.testAT(1000);
}


// ============================================================
// SEND AT COMMAND
// ============================================================

String Modem::command(
    const String& cmd,
    uint32_t timeout
)
{
    String response;

    // Clear stale data
    while (SerialAT.available()) {
        SerialAT.read();
    }

    SerialAT.println(cmd);

    uint32_t start = millis();

    while (millis() - start < timeout) {

        while (SerialAT.available()) {

            char c = SerialAT.read();

            response += c;
        }

        if (
            response.endsWith("OK\r\n") ||
            response.endsWith("ERROR\r\n") ||
            response.indexOf("+CME ERROR") >= 0
        ) {
            break;
        }

        delay(1);
    }

    return response;
}


// ============================================================
// UPDATE
// ============================================================

void Modem::update()
{
    // Modem -> Serial Monitor
    while (SerialAT.available()) {

        Serial.write(
            SerialAT.read()
        );
    }

    // Serial Monitor -> Modem
    while (Serial.available()) {

        SerialAT.write(
            Serial.read()
        );
    }
}
