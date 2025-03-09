#include "firmnginKit.h"
#include <WiFi.h>

#define DEVICE_ID "FNG_YOUR_DEVICE_ID"
#define DEVICE_KEY "FNG_YOUR_DEVICE_KEY"

// WiFi credentials
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

FirmnginKit fngin(DEVICE_ID, DEVICE_KEY);

void callback(Callback& cb)
{
  cb.on(WAIT, [](Payload payload) {
    Serial.println("Event Wait Received");

    Serial.print("Reference ID: ");
    Serial.println(payload.referenceId);

    Serial.print("Amount: ");
    Serial.println(payload.Amount);

    Serial.print("Message ID: ");
    Serial.println(payload.MessageId);

    Serial.print("Option Name: ");
    Serial.println(payload.Optionname);

    Serial.print("Active Session ID: ");
    Serial.println(payload.ActiveSessionID);
  });

  cb.on(IDLE, [](Payload payload) {
    Serial.println("Event IDLE Received");
    Serial.print("Reference ID: ");
    Serial.println(payload.referenceId);
  });

  cb.on(START_SESSION, [](Payload payload) {
    Serial.println("Event START_SESSION Received");
    Serial.print("Reference ID: ");
    Serial.println(payload.referenceId);
  });

  cb.on(END_SESSION, [](Payload payload) {
    Serial.println("Event END_SESSION Received");
    Serial.print("Reference ID: ");
    Serial.println(payload.referenceId);
  });

}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP().toString());

  fngin.setCallback(callback);
  fngin.setDebug(true);
  fngin.begin();
}

void loop()
{
  fngin.loop();
}
