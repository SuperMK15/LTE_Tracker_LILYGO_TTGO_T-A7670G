#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SdFat.h>

class GPS;

class SDCard {
public:
    bool begin();

    bool isReady() const;

    bool startTracking(GPS& gps);
    void stopTracking();

    void update(GPS& gps);

    bool isTracking() const;

    String currentFileName() const;
    uint32_t currentFileSize();

    String listFilesHtml() const;

    bool openFileForDownload(
        const String& filename,
        ExFile& file
    ) const;

private:
    bool initialized = false;
    bool tracking = false;

    ExFile logFile;

    String currentFile;

    uint32_t lastLogTime = 0;

    uint32_t nextFileIndex() const;

    String makeFileName(uint32_t index) const;

    bool writeHeader();
    bool writeGPSRecord(GPS& gps);

    bool isValidDataFileName(
        const String& filename
    ) const;
};
