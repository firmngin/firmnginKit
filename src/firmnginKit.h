#ifndef FIRMNGINKIT_H
#define FIRMNGINKIT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <time.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#define PLATFORM_SUPPORTED true
#define PLATFORM_NAME "ESP8266"
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#define PLATFORM_SUPPORTED true
#define PLATFORM_NAME "ESP32"
#else
#define PLATFORM_SUPPORTED false
#define PLATFORM_NAME "UNKNOWN"
#endif

#define OK "on_ok"
#define WAIT "on_wait"
#define IDLE "on_idle"
#define HOLD "on_hold"
#define START_SESSION "on_start_session"
#define END_SESSION "on_end_session"
#define ERROR "on_error"

class Payload
{
private:
    JsonObject _data;

public:
    Payload(JsonObject data) : _data(data) {}

    String referenceId;
    String EventName;
    int ActiveSessionID;
    int Amount;
    int MessageId;
    String Optionname;
    JsonObject rawData;
    String Metadata;

    bool containsKey(const char *key) const
    {
        return _data.containsKey(key);
    }

    JsonVariant operator[](const char *key) const
    {
        return _data[key];
    }

    void initFromJson(JsonObject data)
    {
        _data = data;
        rawData = data;

        if (data.containsKey("reference_id"))
        {
            referenceId = data["reference_id"].as<String>();
        }

        if (data.containsKey("state"))
        {
            EventName = data["state"].as<String>();
        }

        if (data.containsKey("active_session_id"))
        {
            ActiveSessionID = data["active_session_id"].as<int>();
        }

        if (data.containsKey("amount"))
        {
            Amount = data["amount"].as<int>();
        }

        if (data.containsKey("message_id"))
        {
            MessageId = data["message_id"].as<int>();
        }

        if (data.containsKey("option_name"))
        {
            Optionname = data["option_name"].as<String>();
        }

        if (data.containsKey("metadata"))
        {
            Metadata = data["metadata"].as<String>();
        }
    }
};

typedef void (*EventHandlerFunction)(Payload);

class Callback
{
public:
    String EventName;
    String ReferenceId;
    int ActiveSessionID;
    int Amount;
    int MessageId;
    String Optionname;
    String Metadata; // JSON string containing additional metadata

    void on(const char *event, EventHandlerFunction handler)
    {
        if (EventName == event)
        {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, Metadata);
            JsonObject jsonObj = doc.as<JsonObject>();

            Payload payload(jsonObj);
            payload.initFromJson(jsonObj);

            handler(payload);
        }
    }
};

typedef void (*CallbackFunction)(Callback &);

class FirmnginKit
{
public:
    FirmnginKit(const char *deviceId, const char *deviceKey);

    void begin();
    void loop();
    void setCallback(CallbackFunction callback);
    void setDebug(bool debug);
    void setTimezone(int timezone);
    void setDaylightOffsetSec(int daylightOffsetSec);
    void setNtpServer(const char *ntpServer);
    bool isPlatformSupported();
    FirmnginKit &endSession();

private:
    const char *_deviceId;
    const char *_deviceKey;
    bool _debug;
    bool _noCallback;
    unsigned long _lastMQTTAttempt;
    unsigned long _lastHeartbeat;
    const unsigned long _heartbeatInterval = 20000;
    const char *BASE_URL = "https://apikit.firmngin.cloud";

#if defined(ESP8266) || defined(ESP32)
    WiFiClientSecure _wifiClient;
#else
    WiFiClient _wifiClient;
#endif
    PubSubClient _mqttClient;
    CallbackFunction _callback;
    unsigned long _delayRetryMQTT = 5000;
    int maxRetryMQTT = 3;
    int defaultQos = 1;
    bool initializingDevice(const char *url, String &response, const char *method = "GET", String payload = "");
    void _Debug(String message, bool newLine = true);
    bool connectServer();
    void mqttCallback(char *topic, byte *payload, unsigned int length);
    String setTopicCallback(String deviceId);
    String setTopicHeartbeat(String deviceId);
    String setTopicLastWill(String deviceId);
    String getTopicData(String deviceId);
    void syncTime();
    void sendHeartbeat();
};

#endif // FIRMNGINKIT_H