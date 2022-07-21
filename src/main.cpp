#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.H>
#include <ArduinoJson.h>

#define SENSOR 0

ESP8266WebServer server(80);

uint8_t cmdON[] = {0xA0, 0x01, 0x01, 0xA2};
uint8_t cmdOFF[] = {0xA0, 0x01, 0x00, 0xA1};

bool isRelayOn = false;

uint16_t trigger_counter = 0;
bool isMotionDetected = false;
bool isUserCommand = false;

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
  server.send(200, "text/plain", "Running...");
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not Found");
}

void handleMotion()
{
  if (trigger_counter > 0)
  {
    char result[40];

    if (isMotionDetected == true)
    {
      sprintf(result, "Motion detected! (counter = %d)", trigger_counter);
    }
    else
    {
      sprintf(result, "User command (counter = %d)", trigger_counter);
    }
    server.send(200, "text/plain", result);
  }
  else
    server.send(200, "text/plain", "Idle...");
}

void handleLightsOn()
{
  trigger_counter = 300;
  isUserCommand = true;
  isMotionDetected = false;
}

void handleLightsOff()
{
  turnOff();
  trigger_counter = 0;
  isUserCommand = false;
  isMotionDetected = false;
}

void setup(void)
{
  Serial.begin(9600);

  LittleFS.begin();

  if (LittleFS.exists("/config.json"))
  {
    StaticJsonDocument<512> doc;

    File file = LittleFS.open("/config.json", "r");

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }

    file.close();

    // const char *wifi_ssid = doc["wifi_ssid"];
    // const char *wifi_password = doc["wifi_password"];

    Serial.println();
    Serial.printf("Connecting to SSID: %s", doc["wifi_ssid"].as<const char*>());
    Serial.println();

    pinMode(SENSOR, INPUT);

    WiFi.mode(WIFI_STA);
    WiFi.begin(doc["wifi_ssid"].as<const char *>(), doc["wifi_password"].as<const char *>());
    Serial.println("");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.onNotFound(handleNotFound);

    server.on("/", handleRoot);
    server.on("/motion", handleMotion);
    server.on("/on", handleLightsOn);
    server.on("/off", handleLightsOff);

    server.begin();

    Serial.println("HTTP server started");
  }
  else
  {
    Serial.println("");
    Serial.println("Config file not found!");
    Serial.println("");
  }
  LittleFS.end();
}

void loop(void)
{
  if (trigger_counter > 0)
  {
    trigger_counter--;

    if (trigger_counter == 0)
    {
      turnOff();
      isMotionDetected = false;
      isUserCommand = false;
    }
  }

  server.handleClient();


  if (!isUserCommand)
  {
    uint8_t state = digitalRead(SENSOR);
    if (state > 0)
    {
      isMotionDetected = true;
      trigger_counter = 30;

      if (isRelayOn == false)
      {
        turnOn();
      }
    }
  }
  delay(1000);
}
