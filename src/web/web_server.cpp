#include "web_server.h"

#include "../board_config.h"

#include <WiFi.h>

WebServerManager::WebServerManager(
    GPS& gps,
    Modem& modem,
    SDCard& sd
)
    : server(80),
      gps(gps),
      modem(modem),
      sd(sd)
{
}

void WebServerManager::begin()
{
    WiFi.mode(WIFI_AP);

    WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );

    Serial.println();
    Serial.println("WiFi access point started.");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    server.on(
        "/",
        HTTP_GET,
        [this]() {
            handleRoot();
        }
    );

    server.on(
        "/gps",
        HTTP_GET,
        [this]() {
            handleGPS();
        }
    );

    server.on(
        "/at",
        HTTP_POST,
        [this]() {
            handleAT();
        }
    );

    server.on(
        "/tracker/start",
        HTTP_POST,
        [this]() {
            handleTrackerStart();
        }
    );

    server.on(
        "/tracker/stop",
        HTTP_POST,
        [this]() {
            handleTrackerStop();
        }
    );

    server.on(
        "/download",
        HTTP_GET,
        [this]() {
            handleDownload();
        }
    );

    server.begin();

    Serial.println("Web server started.");
}

void WebServerManager::update()
{
    server.handleClient();
}

void WebServerManager::handleRoot()
{
    String html;

    html.reserve(12000);

    html += R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>EFS LTE Tracker</title>

<style>
body {
    font-family: Arial, sans-serif;
    background: #f4f4f4;
    margin: 0;
    padding: 20px;
    color: #222;
}

.container {
    max-width: 900px;
    margin: auto;
}

h1 {
    margin-bottom: 5px;
}

h2 {
    margin-top: 0;
}

.card {
    background: white;
    border-radius: 10px;
    padding: 20px;
    margin-bottom: 20px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
}

.status {
    font-size: 22px;
    font-weight: bold;
}

.good {
    color: #16803c;
}

.bad {
    color: #c62828;
}

button {
    border: none;
    border-radius: 7px;
    padding: 12px 20px;
    font-size: 16px;
    cursor: pointer;
    margin-right: 8px;
}

.start {
    background: #16803c;
    color: white;
}

.stop {
    background: #c62828;
    color: white;
}

.download {
    background: #1565c0;
    color: white;
    padding: 7px 12px;
    border-radius: 5px;
    text-decoration: none;
}

.grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    gap: 12px;
}

.stat {
    background: #f7f7f7;
    padding: 12px;
    border-radius: 6px;
}

.label {
    color: #666;
    font-size: 13px;
}

.value {
    font-size: 20px;
    margin-top: 4px;
}

.file-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px 0;
    border-bottom: 1px solid #ddd;
}

pre {
    background: #111;
    color: #eee;
    padding: 12px;
    border-radius: 6px;
    overflow-x: auto;
    white-space: pre-wrap;
}

input {
    padding: 10px;
    font-size: 16px;
    width: 70%;
    box-sizing: border-box;
}

.at-button {
    background: #555;
    color: white;
}

.small {
    color: #666;
    font-size: 14px;
}
</style>
</head>

<body>

<div class="container">

<h1>EFS LTE Tracker</h1>
<p class="small">LILYGO T-A7670G + L76K GNSS</p>

<div class="card">

<h2>Tracker</h2>

<div id="tracker-status" class="status">
Loading...
</div>

<p id="tracker-file"></p>
<p id="tracker-size"></p>

<form method="POST" action="/tracker/start" style="display:inline;">
<button class="start" type="submit">
Start Tracking
</button>
</form>

<form method="POST" action="/tracker/stop" style="display:inline;">
<button class="stop" type="submit">
Stop Tracking
</button>
</form>

</div>

<div class="card">

<h2>GNSS</h2>

<div class="grid">

<div class="stat">
<div class="label">Fix</div>
<div id="fix" class="value">-</div>
</div>

<div class="stat">
<div class="label">Satellites</div>
<div id="sats" class="value">-</div>
</div>

<div class="stat">
<div class="label">HDOP</div>
<div id="hdop" class="value">-</div>
</div>

<div class="stat">
<div class="label">Latitude</div>
<div id="lat" class="value">-</div>
</div>

<div class="stat">
<div class="label">Longitude</div>
<div id="lon" class="value">-</div>
</div>

<div class="stat">
<div class="label">Altitude</div>
<div id="alt" class="value">-</div>
</div>

<div class="stat">
<div class="label">Speed</div>
<div id="speed" class="value">-</div>
</div>

<div class="stat">
<div class="label">UTC</div>
<div id="utc" class="value">-</div>
</div>

</div>

<h3>Last NMEA Sentence</h3>
<pre id="nmea">-</pre>

</div>

<div class="card">

<h2>Tracker Files</h2>

<div id="files">
)rawliteral";

    html += sd.listFilesHtml();

    html += R"rawliteral(
</div>

</div>

<div class="card">

<h2>AT Command</h2>

<form id="at-form">

<input
    id="at-command"
    type="text"
    placeholder="AT command"
>

<button
    class="at-button"
    type="submit"
>
Send
</button>

</form>

<pre id="at-response">-</pre>

</div>

</div>

<script>

async function updateGPS()
{
    try {
        const response = await fetch('/gps');
        const data = await response.json();

        document.getElementById('fix').textContent =
            data.fix ? 'YES' : 'NO';

        document.getElementById('sats').textContent =
            data.satellites;

        document.getElementById('hdop').textContent =
            data.hdop;

        document.getElementById('lat').textContent =
            data.latitude;

        document.getElementById('lon').textContent =
            data.longitude;

        document.getElementById('alt').textContent =
            data.altitude + ' m';

        document.getElementById('speed').textContent =
            data.speed + ' km/h';

        document.getElementById('utc').textContent =
            data.utc;

        document.getElementById('nmea').textContent =
            data.nmea;

        document.getElementById('tracker-status').textContent =
            data.tracking ? 'TRACKING' : 'STOPPED';

        document.getElementById('tracker-status').className =
            data.tracking
                ? 'status good'
                : 'status bad';

        document.getElementById('tracker-file').textContent =
            data.file
                ? 'Current file: ' + data.file
                : 'No active file';

        document.getElementById('tracker-size').textContent =
            data.fileSize
                ? 'Size: ' + data.fileSize + ' bytes'
                : '';
    }
    catch (error) {
        console.log(error);
    }
}

document
    .getElementById('at-form')
    .addEventListener('submit', async function(event)
    {
        event.preventDefault();

        const command =
            document.getElementById('at-command').value;

        const response = await fetch('/at', {
            method: 'POST',
            headers: {
                'Content-Type':
                    'application/x-www-form-urlencoded'
            },
            body:
                'command=' +
                encodeURIComponent(command)
        });

        const text = await response.text();

        document.getElementById('at-response')
            .textContent = text;
    });

setInterval(updateGPS, 1000);

updateGPS();

</script>

</body>
</html>
)rawliteral";

    server.send(
        200,
        "text/html",
        html
    );
}

String WebServerManager::gpsHTML()
{
    TinyGPSPlus& data = gps.data();

    String json = "{";

    json += "\"fix\":";
    json += gps.hasFix() ? "true" : "false";

    json += ",\"satellites\":";
    if (data.satellites.isValid()) {
        json += String(data.satellites.value());
    } else {
        json += "0";
    }

    json += ",\"hdop\":";
    if (data.hdop.isValid()) {
        json += String(data.hdop.hdop(), 2);
    } else {
        json += "\"N/A\"";
    }

    json += ",\"latitude\":";
    if (data.location.isValid()) {
        json += String(data.location.lat(), 7);
    } else {
        json += "\"N/A\"";
    }

    json += ",\"longitude\":";
    if (data.location.isValid()) {
        json += String(data.location.lng(), 7);
    } else {
        json += "\"N/A\"";
    }

    json += ",\"altitude\":";
    if (data.altitude.isValid()) {
        json += String(data.altitude.meters(), 2);
    } else {
        json += "\"N/A\"";
    }

    json += ",\"speed\":";
    if (data.speed.isValid()) {
        json += String(data.speed.kmph(), 2);
    } else {
        json += "\"N/A\"";
    }

    json += ",\"utc\":\"";

    if (
        data.date.isValid() &&
        data.time.isValid()
    ) {
        char timestamp[32];

        snprintf(
            timestamp,
            sizeof(timestamp),
            "%04d-%02d-%02d %02d:%02d:%02d",
            data.date.year(),
            data.date.month(),
            data.date.day(),
            data.time.hour(),
            data.time.minute(),
            data.time.second()
        );

        json += timestamp;
    }
    else {
        json += "N/A";
    }

    json += "\"";

    json += ",\"nmea\":\"";

    String nmea = gps.lastSentence();

    nmea.replace("\\", "\\\\");
    nmea.replace("\"", "\\\"");
    nmea.replace("\r", "");
    nmea.replace("\n", "");

    json += nmea;

    json += "\"";

    json += ",\"tracking\":";
    json += sd.isTracking() ? "true" : "false";

    json += ",\"file\":\"";

    String filename = sd.currentFileName();

    filename.replace("\\", "\\\\");
    filename.replace("\"", "\\\"");

    json += filename;

    json += "\"";

    json += ",\"fileSize\":";
    json += String(sd.currentFileSize());

    json += "}";

    return json;
}

void WebServerManager::handleGPS()
{
    server.send(
        200,
        "application/json",
        gpsHTML()
    );
}

void WebServerManager::handleAT()
{
    if (!server.hasArg("command")) {
        server.send(
            400,
            "text/plain",
            "Missing command"
        );

        return;
    }

    String command = server.arg("command");

    String response = modem.command(
        command,
        5000
    );

    server.send(
        200,
        "text/plain",
        response
    );
}

void WebServerManager::handleTrackerStart()
{
    bool success = sd.startTracking(gps);

    if (!success) {
        server.send(
            500,
            "text/html",
            "<h1>Failed to start tracking</h1>"
            "<p>Check that an SD card is inserted.</p>"
            "<a href='/'>Back</a>"
        );

        return;
    }

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(
        303
    );
}

void WebServerManager::handleTrackerStop()
{
    sd.stopTracking();

    server.sendHeader(
        "Location",
        "/"
    );

    server.send(
        303
    );
}

void WebServerManager::handleDownload()
{
    if (!server.hasArg("file")) {
        server.send(
            400,
            "text/plain",
            "Missing file"
        );

        return;
    }

    String filename = server.arg("file");

    ExFile file;

    if (!sd.openFileForDownload(filename, file)) {
        server.send(
            404,
            "text/plain",
            "File not found"
        );

        return;
    }

    uint64_t fileSize = file.size();

    server.sendHeader(
        "Content-Disposition",
        "attachment; filename=\"" + filename + "\""
    );

    server.setContentLength(fileSize);

    server.send(
        200,
        "text/csv",
        ""
    );

    uint8_t buffer[1024];

    while (file.available()) {
        size_t bytesRead = file.read(
            buffer,
            sizeof(buffer)
        );

        if (bytesRead == 0) {
            break;
        }

        server.client().write(
            buffer,
            bytesRead
        );
    }

    file.close();
}
