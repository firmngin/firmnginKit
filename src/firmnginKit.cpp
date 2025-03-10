#include "firmnginKit.h"

#define MQTT_TOPIC_PREFIX "fngin/"
#define SECURE_MQTT
const char *NTP_SERVER = "pool.ntp.org";
int GMT_OFFSET_SEC = 7 * 3600; // Default timezone for Indonesia +7
int DAYLIGHT_OFFSET_SEC = 0;
#ifdef SECURE_MQTT
#include "esp_tls.h"
#endif

String MQ_SERVER = "";
int MQ_PORT = 0;
String MQ_USERNAME = "";
String MQ_PASSWORD = "";

static const char ca_cert_cf[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEqDCCA5CgAwIBAgIUNlBmbnamMRzUS+l1bWNCkIgQdZkwDQYJKoZIhvcNAQEL
BQAwgYsxCzAJBgNVBAYTAlVTMRkwFwYDVQQKExBDbG91ZEZsYXJlLCBJbmMuMTQw
MgYDVQQLEytDbG91ZEZsYXJlIE9yaWdpbiBTU0wgQ2VydGlmaWNhdGUgQXV0aG9y
aXR5MRYwFAYDVQQHEw1TYW4gRnJhbmNpc2NvMRMwEQYDVQQIEwpDYWxpZm9ybmlh
MB4XDTI1MDIyNjE2MjUwMFoXDTQwMDIyMzE2MjUwMFowYjEZMBcGA1UEChMQQ2xv
dWRGbGFyZSwgSW5jLjEdMBsGA1UECxMUQ2xvdWRGbGFyZSBPcmlnaW4gQ0ExJjAk
BgNVBAMTHUNsb3VkRmxhcmUgT3JpZ2luIENlcnRpZmljYXRlMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz63+sgykyu5XprAp8EtUcyNBhGk/KsqJePht
gAQDbfoT57UOEYRHXOSfLg2OlLjfJYfmrjO8Q8X8D+Gh84Ar6Rj3DNw+hAn8qGKV
klby7xQ8kcE3TiNwRxg+ZHidmzhfgGXknJf1EbifGIDEIkZ8konBegX+T4EN1G4F
vsnffxWG572Dz81tUNLFknTABHmHZIEbzXTQ6aN9LM91IdvByc36pJifMHmx5z3P
x1Qs69bsSDM+ExxzBGcRfvMWeWP0n7y2DPvnK6LICEln9rXXxkTF71sSKjGPt/P/
WbZUrGavIR+b6MfNJ1rZ0/roeO/eQPdh648CQZcQjDb905r4jwIDAQABo4IBKjCC
ASYwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcD
ATAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBQSRnViDoUWLtBLTqK6j+EBWNehzTAf
BgNVHSMEGDAWgBQk6FNXXXw0QIep65TbuuEWePwppDBABggrBgEFBQcBAQQ0MDIw
MAYIKwYBBQUHMAGGJGh0dHA6Ly9vY3NwLmNsb3VkZmxhcmUuY29tL29yaWdpbl9j
YTArBgNVHREEJDAighAqLmZpcm1uZ2luLmNsb3Vkgg5maXJtbmdpbi5jbG91ZDA4
BgNVHR8EMTAvMC2gK6AphidodHRwOi8vY3JsLmNsb3VkZmxhcmUuY29tL29yaWdp
bl9jYS5jcmwwDQYJKoZIhvcNAQELBQADggEBAIjcYZeL65Dg4bpwuwkaS/UajXNK
3+lAjrZ6TFwb2QiiJzLmg8/QzVOMs1L5+eXbiv5Z/akQlyaSY2aDQVbIZv/tucVt
F/C+drkxZ0lIiBzD+Vh29wTGGUv6JROVq9Zcu/1ECy5F2u+YCnvlXsRNcIYOvb0s
4EPI6vqIQ23nk565ogrGf0GS8PfVvaBxtfxLJbOzU5B6ng1JUrmsWwhD+Qq3dQQM
y/X1/tXUNXNlQnbZOe7JNAXmGdkQdfW726OjJrD7DnrllSjACUJcvFPRWREQXSxx
nWn774BCBurLwbJdfDuf12okyC+PPyBLE6gnzveA+I9qch72pPE3nr7u0Rg=
-----END CERTIFICATE-----
)EOF";

FirmnginKit::FirmnginKit(const char *deviceId, const char *deviceKey)
    : _deviceId(deviceId),
      _deviceKey(deviceKey),
      _debug(false),
      _noCallback(false),
      _lastMQTTAttempt(0),
      _mqttClient(_wifiClient)
{
    if (!PLATFORM_SUPPORTED)
    {
        Serial.println("ERROR: Platform board not supported");
        Serial.print("firmnginKit only supported ");
        Serial.print("ESP8266 and ESP32, not ");
        Serial.println(PLATFORM_NAME);
    }
}

bool FirmnginKit::initializingDevice(const char *url, String &response, const char *method, String payload)
{

    bool success = false;
#if defined(ESP8266)
    WiFiClientSecure client;
    client.setTrustAnchors(new BearSSL::X509List(ca_cert_cf));
    client.setInsecure();

    HTTPClient http;
    http.begin(client, url);

    int httpCode = 0;
    if (strcmp(method, "GET") == 0)
    {
        httpCode = http.GET();
    }
    else if (strcmp(method, "POST") == 0)
    {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.POST(payload);
    }
    else if (strcmp(method, "PUT") == 0)
    {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.PUT(payload);
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        httpCode = http.sendRequest("DELETE", payload);
    }

    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            response = http.getString();
            success = true;
        }
    }
    else
    {
        if (_debug)
        {
            Serial.print("HTTP Request failed, error: ");
            Serial.println(http.errorToString(httpCode));
        }
    }

    http.end();

#elif defined(ESP32)
    WiFiClientSecure client;
    client.setCACert(ca_cert_cf);
    client.setInsecure(); // Tambahkan ini untuk debugging

    HTTPClient http;
    http.begin(url);

    int httpCode = 0;
    if (strcmp(method, "GET") == 0)
    {
        httpCode = http.GET();
    }
    else if (strcmp(method, "POST") == 0)
    {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.POST(payload);
    }
    else if (strcmp(method, "PUT") == 0)
    {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.PUT(payload);
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        httpCode = http.sendRequest("DELETE", payload);
    }

    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            response = http.getString();
            Serial.println("Device initialized");
            success = true;
        }
    }
    else
    {
        if (_debug)
        {
            Serial.print("Initializing device failed, error: ");
            Serial.println(http.errorToString(httpCode));
            return false;
        }
    }

    http.end();
#endif

    return success;
}

String FirmnginKit::setTopicCallback(String deviceId)
{
    String topic = MQTT_TOPIC_PREFIX;
    topic += deviceId;
    topic += "/callback";
    return topic;
}

String FirmnginKit::getTopicData(String deviceId)
{
    String topic = MQTT_TOPIC_PREFIX;
    topic += deviceId;
    topic += "/dt";
    return topic;
}

void FirmnginKit::begin()
{
    if (!PLATFORM_SUPPORTED)
    {
        Serial.println("ERROR: Platform board not supported");
        Serial.print("firmnginKit only supported ");
        Serial.print("ESP8266 and ESP32, not ");
        Serial.println(PLATFORM_NAME);
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("ERROR: WiFi not connected");
        Serial.println("Restarting ESP...");
        delay(2000);
        ESP.restart();
        return;
    }

    syncTime();

    String response;
    Serial.println("Initializing device");
    if (initializingDevice(("https://dev.firmngin.cloud/v1/device/init/" + String(_deviceId)).c_str(), response))
    {

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, response);

        if (!error)
        {
            if (doc.containsKey("srv"))
            {
                MQ_SERVER = doc["srv"].as<const char *>();
            }
            if (doc.containsKey("prt"))
            {
                MQ_PORT = doc["prt"].as<int>();
            }
            if (doc.containsKey("usr"))
            {
                MQ_USERNAME = doc["usr"].as<const char *>();
            }
            if (doc.containsKey("pwd"))
            {
                MQ_PASSWORD = doc["pwd"].as<const char *>();
            }
        }
        else
        {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
        }
    }
    else
    {
        Serial.println("Failed to initialize device");
        Serial.println("Restarting ESP...");
        delay(2000);
        ESP.restart();
    }

#if defined(ESP8266)
    _wifiClient.setInsecure();
#elif defined(ESP32)
    _wifiClient.setCACert(ca_cert_cf);
    _wifiClient.setInsecure(); // Tambahkan ini untuk debugging, hapus di produksi
#endif

    _mqttClient.setServer(MQ_SERVER.c_str(), MQ_PORT);
    _mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                            { this->mqttCallback(topic, payload, length); });
    _mqttClient.setBufferSize(2048);
    _mqttClient.setKeepAlive(60);
    _mqttClient.setSocketTimeout(30);
}

void FirmnginKit::setTimezone(int timezone)
{
    if (timezone < -12 || timezone > 12)
    {
        Serial.println("ERROR: Invalid timezone");
        return;
    }

    GMT_OFFSET_SEC = timezone * 3600;
}

void FirmnginKit::setNtpServer(const char *ntpServer)
{
    NTP_SERVER = ntpServer;
}

void FirmnginKit::setDaylightOffsetSec(int daylightOffsetSec)
{
    DAYLIGHT_OFFSET_SEC = daylightOffsetSec;
}

void FirmnginKit::syncTime()
{
    Serial.println("Syncing time");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2)
    {
        delay(10);
        now = time(nullptr);
    }

    if (_debug)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            Serial.print("Current time: ");
            Serial.println(asctime(&timeinfo));
        }
    }
}

void FirmnginKit::loop()
{
    if (!PLATFORM_SUPPORTED)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        if (_debug)
        {
            Serial.println("WiFi not connected");
        }
        return;
    }

    static unsigned long lastReconnectAttempt = 0;
    static int reconnectCount = 0;
    static int backoffDelay = 5000;
    static bool firstConnect = true;

    if (!_mqttClient.connected())
    {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > backoffDelay)
        {
            lastReconnectAttempt = now;
            reconnectCount++;
            backoffDelay = min(backoffDelay * 2, 60000);

            if (_debug && !firstConnect)
            {
                Serial.println("Connection lost, attempting to reconnect...");
            }

            if (connectServer())
            {
                reconnectCount = 0;
                backoffDelay = 5000;
                firstConnect = false;
            }
        }
    }
    else
    {
        _mqttClient.loop();
    }
}

void FirmnginKit::_Debug(String message, bool newLine)
{
    if (_debug)
    {
        if (newLine)
        {
            Serial.println(message);
        }
        else
        {
            Serial.print(message);
        }
    }
}

void FirmnginKit::setCallback(CallbackFunction callback)
{
    _callback = callback;
}

void FirmnginKit::setDebug(bool debug)
{
    _debug = debug;
}

bool FirmnginKit::isPlatformSupported()
{
    return PLATFORM_SUPPORTED;
}

bool FirmnginKit::connectServer()
{
    int retryCount = 0;
    while (!_mqttClient.connected() && retryCount < maxRetryMQTT)
    {
        unsigned long now = millis();
        if (now - _lastMQTTAttempt >= _delayRetryMQTT)
        {
            _lastMQTTAttempt = now;
            Serial.print("Connecting to server (attempt ");
            Serial.print(retryCount + 1);
            Serial.println(")");

            if (MQ_USERNAME.length() > 0 && MQ_PASSWORD.length() > 0)
            {
                if (_mqttClient.connect(_deviceId, MQ_USERNAME.c_str(), MQ_PASSWORD.c_str()))
                {
                    _mqttClient.subscribe(getTopicData(_deviceId).c_str(), defaultQos);
                    Serial.println("   __ _                            _             _    _ _   ");
                    Serial.println("  / _(_)_ __ _ __ ___  _ __   __ _(_)_ __       | | _(_) |_ ");
                    Serial.println(" | |_| | '__| '_ ` _ \\| '_ \\ / _` | | '_ \\ _____| |/ / | __|");
                    Serial.println(" |  _| | |  | | | | | | | | | (_| | | | | |_____|   <| | |_ ");
                    Serial.println(" |_| |_|_|  |_| |_| |_|_| |_|\\__, |_|_| |_|     |_|\\_\\_|\\__|");
                    Serial.println("                             |___/                           ");
                    Serial.println("\nConnected Successfully, let's build amazing things");
                    return true;
                }
                else
                {
                    Serial.println("Connect to Server failed, rc=" + String(_mqttClient.state()));
                    retryCount++;
                }
            }
            else
            {
                Serial.println("Connect to Server failed, rc=" + String(_mqttClient.state()));
                retryCount++;
            }
        }
    }

    if (!_mqttClient.connected())
    {
        Serial.println("Failed to connect to Server after " + String(maxRetryMQTT) + " attempts");
        Serial.println("Restarting ESP...");
        delay(1000);
        ESP.restart();
    }

    return false;
}

void FirmnginKit::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // if (_debug)
    // {
    //     Serial.print("Message arrived [");
    //     Serial.print(topic);
    //     Serial.print("] ");
    //     for (unsigned int i = 0; i < length; i++)
    //     {
    //         Serial.print(String((char)payload[i]));
    //     }
    //     Serial.println();
    // }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error)
    {
        if (_debug)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
        }
        return;
    }

    Callback data;
    const char *state = doc["state"];
    data.EventName = state;

    const char *reference_id = doc["reference_id"];
    data.ReferenceId = reference_id;

    data.Amount = doc["amount"];
    data.MessageId = doc["message_id"];
    data.ActiveSessionID = doc["active_session_id"];

    const char *option_name = doc["option_name"];
    data.Optionname = option_name;

    // Get metadata
    String metadataJson;
    serializeJson(doc, metadataJson);
    data.Metadata = metadataJson;

    if (_callback)
    {
        _callback(data);
    }

    if (!_noCallback)
    {
        if (_mqttClient.connected())
        {
            DynamicJsonDocument doc2(256);
            doc2["state"] = OK;
            doc2["active_session_id"] = data.ActiveSessionID;
            doc2["message_id"] = data.MessageId;
            String payload2;
            serializeJson(doc2, payload2);
            String topic2 = setTopicCallback(_deviceId);
            _mqttClient.publish(topic2.c_str(), payload2.c_str());
        }
        else
        {
            if (_debug)
            {
                Serial.print("Server not connected. Cannot sent callback.");
            }
        }
    }
}

FirmnginKit &FirmnginKit::endSession()
{
    String topic = MQTT_TOPIC_PREFIX;
    topic += _deviceId;

    if (_mqttClient.connected())
    {
        DynamicJsonDocument doc(256);
        doc["state"] = END_SESSION;
        String payload;
        serializeJson(doc, payload);

        _mqttClient.publish(topic.c_str(), payload.c_str());
    }
    else
    {
        if (_debug)
        {
            Serial.print("Server not connected. Cannot sent release message.");
        }
    }
    return *this;
}