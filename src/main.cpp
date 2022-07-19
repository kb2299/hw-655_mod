#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "Kropotkina";
const char *password = "X3WUN68T";

bool isRelayOn = false;
bool isMotionDetected = false;
uint8_t motion_trigger_counter = 0;

ESP8266WebServer server(80);
HTTPClient http;

void turnOn()
{
  Serial.write("\xa0\x01"); // byte sequence for opening relay
  Serial.write(0x00);       // apparently because of the 0x00
  Serial.write(0xa1);       // you need to send on multiple lines
  isRelayOn = true;
}

void turnOff()
{
  Serial.write("\xa0\x01\x01\xa2"); // byte sequence for closing relay
  isRelayOn = false;
}

void toggle()
{
  if (isRelayOn)
  {
    turnOn();
  }
  else
  {
    turnOff();
  }
}

void handleRoot()
{
  server.send(200, "text/plain", "Hello, the Garage Door gateway is ready");
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not Found");
}

void handleOpen()
{
  turnOn();
  server.send(200, "text/plain", "Relay ON");
}

void handleClose()
{
  turnOff();
  server.send(200, "text/plain", "Relay Off");
}

void handleMotion()
{
  if (isMotionDetected == true)
    server.send(200, "text/plain", "Motion detected!");
  else
    server.send(200, "text/plain", "No motion...");
}

void setup(void)
{
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/1", handleOpen);
  server.on("/0", handleClose);
  server.on("/222", handleMotion);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
}

byte *buffer[5];

void loop(void)
{
  server.handleClient();

  if (motion_trigger_counter > 0)
  {
    if (motion_trigger_counter == 1)
    {
      turnOff();
    }
    motion_trigger_counter--;
  }
  else
  {
    isMotionDetected = false;
  }

  size_t len = Serial.available();
  uint8_t sbuf[len];
  Serial.readBytes(sbuf, len);
  if (sbuf != NULL && sbuf[0] == 1)
  {
    isMotionDetected = true;
    motion_trigger_counter = 30;
    turnOn();
  }
  delay(1000);
}
