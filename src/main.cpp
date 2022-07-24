#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.H>
#include <ArduinoJson.h>

#define SENSOR 0
#define LED 13
ESP8266WebServer server(80);

uint8_t cmdON[] = {0xA0, 0x01, 0x01, 0xA2};
uint8_t cmdOFF[] = {0xA0, 0x01, 0x00, 0xA1};

bool isRelayOn = false;

uint16_t trigger_counter = 0;
bool isMotionDetected = false;
// bool isUserCommand = false;

void turnOn()
{
  Serial.write(cmdON, 4);
  isRelayOn = true;
}

void turnOff()
{
  Serial.write(cmdOFF, 4);
  isRelayOn = false;
}

void handleRoot()
{
  String path = "/index.html";

  String contentType = mime::getContentType(path);
  File file = SPIFFS.open("/index.html", "r");
  server.streamFile(file, contentType);
  file.close();
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not Found");
}

void handleStatus()
{
  StaticJsonDocument<200> doc;
  doc["relay"] = isRelayOn;
  doc["motion_detected"] = isMotionDetected;
  doc["counter"] = trigger_counter;
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleLightsOn()
{
  turnOn();
  trigger_counter = 300;
  // isUserCommand = true;
  isMotionDetected = false;
  server.send(200, "text/plain", "200");
}

void handleLightsOff()
{
  turnOff();
  trigger_counter = 0;
  // isUserCommand = false;
  isMotionDetected = false;

  server.send(200, "text/plain", "200");
}

void setup(void)
{

  Serial.begin(9600);
  // Serial.begin(9600);
  pinMode(SENSOR, INPUT);

  SPIFFS.begin();

  if (SPIFFS.exists("/config.json"))
  {
    StaticJsonDocument<512> doc;

    File file = SPIFFS.open("/config.json", "r");

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      // Serial.print(F("deserializeJson() failed: "));
      // Serial.println(error.f_str());
    }

    file.close();

    // const char *wifi_ssid = doc["wifi_ssid"];
    // const char *wifi_password = doc["wifi_password"];

    // Serial.println();
    // Serial.printf("Connecting to SSID: %s", doc["wifi_ssid"].as<const char *>());
    // Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.begin(doc["wifi_ssid"].as<const char *>(), doc["wifi_password"].as<const char *>());
    // Serial.println("");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      // Serial.print(".");
    }

    // Serial.println("");
    // Serial.print("Connected!");
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());

    server.onNotFound(handleNotFound);

    server.on("/", handleRoot);
    // server.on("/motion", handleMotion);
    server.on("/on", handleLightsOn);
    server.on("/off", handleLightsOff);
    server.on("/status", handleStatus);

    server.begin();
  }
}

void loop(void)
{
  server.handleClient();

  if (trigger_counter > 0)
  {
    trigger_counter--;

    if (trigger_counter == 0)
    {
      turnOff();
    }
  }

  // if (trigger_counter < 2)
  {
    int state = digitalRead(SENSOR);
    if (state > 0)
    {
      isMotionDetected = true;
      trigger_counter = 60 * 2;

      if (isRelayOn == false)
      {
        turnOn();
      }
    }
    else
    {
      isMotionDetected = true;
    }
  }

  delay(500);
}
