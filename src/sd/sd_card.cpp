#include "sd_card.h"

#include "../board_config.h"
#include "../gps/gps.h"

#include <SPI.h>

SdExFat exfat;

bool SDCard::begin()
{
    Serial.println();
    Serial.println("Initializing SD card...");

    pinMode(PERIPHERAL_POWER_PIN, OUTPUT);
    digitalWrite(PERIPHERAL_POWER_PIN, HIGH);

    delay(500);

    Serial.println("Peripheral power enabled.");

    Serial.printf(
        "SD pins: SCK=%d MISO=%d MOSI=%d CS=%d\n",
        SD_SCK_PIN,
        SD_MISO_PIN,
        SD_MOSI_PIN,
        SD_CS_PIN
    );

    SPI.begin(
        SD_SCK_PIN,
        SD_MISO_PIN,
        SD_MOSI_PIN,
        SD_CS_PIN
    );

    Serial.println("SPI initialized.");

    if (!exfat.begin(
            SdSpiConfig(
                SD_CS_PIN,
                SHARED_SPI,
                SD_SCK_MHZ(10)
            )
        )) {

        Serial.println("ERROR: SD initialization failed.");

        initialized = false;
        return false;
    }

    Serial.println("SD card initialized.");

    uint8_t fatType = exfat.vol()->fatType();

    if (fatType > 32) {
        Serial.println("Filesystem: exFAT");
    }
    else {
        Serial.printf(
            "Filesystem: FAT%d\n",
            fatType
        );
    }

    Serial.printf(
        "SD card size: %llu MB\n",
        exfat.card()->sectorCount() * 512ULL /
        (1024ULL * 1024ULL)
    );

    if (!exfat.exists(TRACKER_DIRECTORY)) {
        Serial.println("Creating tracker_data directory...");

        if (!exfat.mkdir(TRACKER_DIRECTORY)) {
            Serial.println(
                "ERROR: Failed to create tracker_data directory."
            );

            initialized = false;
            return false;
        }
    }

    initialized = true;

    Serial.println("SD card ready.");

    Serial.printf(
        "Next tracker file: %s\n",
        makeFileName(nextFileIndex()).c_str()
    );

    return true;
}

bool SDCard::isReady() const
{
    return initialized;
}

bool SDCard::startTracking(GPS& gps)
{
    if (!initialized) {
        Serial.println(
            "Cannot start tracking: SD card unavailable."
        );

        return false;
    }

    if (tracking) {
        return true;
    }

    uint32_t index = nextFileIndex();

    currentFile = makeFileName(index);

    Serial.printf(
        "Creating tracker file: %s\n",
        currentFile.c_str()
    );

    logFile = exfat.open(
        currentFile.c_str(),
        O_WRONLY | O_CREAT | O_TRUNC
    );

    if (!logFile) {
        Serial.printf(
            "Failed to create tracker file: %s\n",
            currentFile.c_str()
        );

        currentFile = "";

        return false;
    }

    if (!writeHeader()) {
        logFile.close();
        currentFile = "";

        return false;
    }

    tracking = true;
    lastLogTime = millis();

    writeGPSRecord(gps);

    Serial.printf(
        "Tracking started: %s\n",
        currentFile.c_str()
    );

    return true;
}

void SDCard::stopTracking()
{
    if (!tracking) {
        return;
    }

    logFile.sync();
    logFile.close();

    tracking = false;

    Serial.printf(
        "Tracking stopped: %s\n",
        currentFile.c_str()
    );
}

void SDCard::update(GPS& gps)
{
    if (!tracking) {
        return;
    }

    uint32_t now = millis();

    if (now - lastLogTime < TRACKER_INTERVAL_MS) {
        return;
    }

    lastLogTime = now;

    writeGPSRecord(gps);
}

bool SDCard::isTracking() const
{
    return tracking;
}

String SDCard::currentFileName() const
{
    return currentFile;
}

uint32_t SDCard::currentFileSize()
{
    if (currentFile.length() == 0) {
        return 0;
    }

    if (tracking && logFile) {
        uint64_t size = logFile.size();

        if (size > UINT32_MAX) {
            return UINT32_MAX;
        }

        return static_cast<uint32_t>(size);
    }

    ExFile file = exfat.open(
        currentFile.c_str(),
        O_RDONLY
    );

    if (!file) {
        return 0;
    }

    uint64_t size = file.size();

    file.close();

    if (size > UINT32_MAX) {
        return UINT32_MAX;
    }

    return static_cast<uint32_t>(size);
}

uint32_t SDCard::nextFileIndex() const
{
    if (!initialized) {
        return 0;
    }

    ExFile directory = exfat.open(
        TRACKER_DIRECTORY,
        O_RDONLY
    );

    if (!directory || !directory.isDirectory()) {
        return 0;
    }

    uint32_t highestIndex = 0;
    bool foundFile = false;

    ExFile file = directory.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            char nameBuffer[256];

            if (file.getName(
                    nameBuffer,
                    sizeof(nameBuffer)
                )) {

                String name = String(nameBuffer);

                int slash = name.lastIndexOf('/');

                if (slash >= 0) {
                    name = name.substring(slash + 1);
                }

                if (
                    name.startsWith("data") &&
                    name.endsWith(".csv")
                ) {
                    String number = name.substring(
                        4,
                        name.length() - 4
                    );

                    bool validNumber =
                        number.length() > 0;

                    for (
                        size_t i = 0;
                        i < number.length();
                        i++
                    ) {
                        if (!isDigit(number[i])) {
                            validNumber = false;
                            break;
                        }
                    }

                    if (validNumber) {
                        uint32_t index = number.toInt();

                        if (
                            !foundFile ||
                            index > highestIndex
                        ) {
                            highestIndex = index;
                        }

                        foundFile = true;
                    }
                }
            }
        }

        file.close();

        file = directory.openNextFile();
    }

    directory.close();

    if (!foundFile) {
        return 0;
    }

    return highestIndex + 1;
}

String SDCard::makeFileName(uint32_t index) const
{
    return String(TRACKER_DIRECTORY)
        + "/data"
        + String(index)
        + ".csv";
}

bool SDCard::writeHeader()
{
    if (!logFile) {
        return false;
    }

    logFile.println(
        "timestamp,latitude,longitude,altitude_m,"
        "speed_kmph,satellites,hdop,course_deg"
    );

    return logFile.sync();
}

bool SDCard::writeGPSRecord(GPS& gps)
{
    if (!logFile) {
        return false;
    }

    TinyGPSPlus& data = gps.data();

    if (
        data.date.isValid() &&
        data.time.isValid()
    ) {
        char timestamp[32];

        snprintf(
            timestamp,
            sizeof(timestamp),
            "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            data.date.year(),
            data.date.month(),
            data.date.day(),
            data.time.hour(),
            data.time.minute(),
            data.time.second(),
            data.time.centisecond() * 10
        );

        logFile.print(timestamp);
    }
    else {
        logFile.print("NO_TIME");
    }

    logFile.print(",");

    if (data.location.isValid()) {
        logFile.print(data.location.lat(), 7);
        logFile.print(",");
        logFile.print(data.location.lng(), 7);
    }
    else {
        logFile.print(",");
    }

    logFile.print(",");

    if (data.altitude.isValid()) {
        logFile.print(data.altitude.meters(), 2);
    }

    logFile.print(",");

    if (data.speed.isValid()) {
        logFile.print(data.speed.kmph(), 2);
    }

    logFile.print(",");

    if (data.satellites.isValid()) {
        logFile.print(data.satellites.value());
    }

    logFile.print(",");

    if (data.hdop.isValid()) {
        logFile.print(data.hdop.hdop(), 2);
    }

    logFile.print(",");

    if (data.course.isValid()) {
        logFile.print(data.course.deg(), 2);
    }

    logFile.println();

    return logFile.sync();
}

bool SDCard::isValidDataFileName(
    const String& filename
) const
{
    if (
        filename.length() < 9 ||
        !filename.startsWith("data") ||
        !filename.endsWith(".csv")
    ) {
        return false;
    }

    String number = filename.substring(
        4,
        filename.length() - 4
    );

    if (number.length() == 0) {
        return false;
    }

    for (
        size_t i = 0;
        i < number.length();
        i++
    ) {
        if (!isDigit(number[i])) {
            return false;
        }
    }

    return true;
}

bool SDCard::openFileForDownload(
    const String& filename,
    ExFile& file
) const
{
    if (!initialized) {
        return false;
    }

    if (!isValidDataFileName(filename)) {
        return false;
    }

    String path =
        String(TRACKER_DIRECTORY)
        + "/"
        + filename;

    file = exfat.open(
        path.c_str(),
        O_RDONLY
    );

    return file;
}

String SDCard::listFilesHtml() const
{
    if (!initialized) {
        return "<p>SD card unavailable.</p>";
    }

    ExFile directory = exfat.open(
        TRACKER_DIRECTORY,
        O_RDONLY
    );

    if (!directory || !directory.isDirectory()) {
        return "<p>Tracker directory unavailable.</p>";
    }

    String html;

    ExFile file = directory.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            char nameBuffer[256];

            if (file.getName(
                    nameBuffer,
                    sizeof(nameBuffer)
                )) {

                String name = String(nameBuffer);

                int slash = name.lastIndexOf('/');

                if (slash >= 0) {
                    name = name.substring(slash + 1);
                }

                if (isValidDataFileName(name)) {
                    html += "<div class='file-row'>";

                    html += "<span>";
                    html += name;
                    html += " &nbsp; ";
                    html += String(file.size());
                    html += " bytes";
                    html += "</span>";

                    html += "<a class='download' "
                            "href='/download?file=";
                    html += name;
                    html += "'>Download</a>";

                    html += "</div>";
                }
            }
        }

        file.close();

        file = directory.openNextFile();
    }

    directory.close();

    if (html.length() == 0) {
        return "<p>No tracker files yet.</p>";
    }

    return html;
}
