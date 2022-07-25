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

void blinkError()
{
  for (int i = 0; i < 5; ++i)
  {
    digitalWrite(LED, 1);
    delay(150);
    digitalWrite(LED, 0);
    delay(100);
    digitalWrite(LED, 1);
    delay(150);
    digitalWrite(LED, 0);
    delay(100);
    digitalWrite(LED, 1);
    delay(150);
    digitalWrite(LED, 0);
    delay(500);
  }
}
void blinkOn()
{
  digitalWrite(BUILTIN_LED, 0);
}
void blinkOff()
{
  digitalWrite(BUILTIN_LED, 1);
}

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
  isMotionDetected = false;
  server.send(200, "text/plain", "200");
}

void handleLightsOff()
{
  turnOff();
  trigger_counter = 0;
  isMotionDetected = false;

  server.send(200, "text/plain", "200");
}

void setup(void)
{
  Serial.begin(9600);

  pinMode(SENSOR, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);

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

    WiFi.mode(WIFI_STA);
    WiFi.begin(doc["wifi_ssid"].as<const char *>(), doc["wifi_password"].as<const char *>());
    // Serial.println("");

    while (WiFi.status() != WL_CONNECTED)
    {
      blinkOn();
      delay(1000);
      blinkOff();
      Serial.print(".");
    }

    blinkOff();

    Serial.println("");
    Serial.print("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.onNotFound(handleNotFound);

    server.on("/", handleRoot);
    server.on("/on", handleLightsOn);
    server.on("/off", handleLightsOff);
    server.on("/status", handleStatus);

    server.begin();
    blinkOff();
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
    isMotionDetected = false;
  }

  delay(500);
}
