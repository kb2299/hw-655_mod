#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define SENSOR 0

const char *ssid = "Kropotkina";
const char *password = "X3WUN68T";

bool isRelayOn = false;
bool isMotionDetected = false;
uint8_t motion_trigger_counter = 0;

ESP8266WebServer server(80);
HTTPClient http;
uint8_t cmdON[] = {0xA0, 0x01, 0x01, 0xA2};
uint8_t cmdOFF[] = {0xA0, 0x01, 0x00, 0xA1};

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

// void toggle()
// {
//   if (isRelayOn)
//   {
//     turnOn();
//   }
//   else
//   {
//     turnOff();
//   }
// }

void handleRoot()
{
  server.send(200, "text/plain", "Running...");
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not Found");
}

// void handleOpen()
// {
//   turnOn();
//   server.send(200, "text/plain", "Relay ON");
// }

// void handleClose()
// {
//   turnOff();
//   server.send(200, "text/plain", "Relay Off");
// }

void handleMotion()
{
  if (isMotionDetected == true)
  {
    // char  result[40];
    // sprintf(result, "Motion detected! (counter = %d)", motion_trigger_counter);
    server.send(200, "text/plain", "Motion detected!");
  }
  else
    server.send(200, "text/plain", "No motion...");
}

void setup(void)
{
  Serial.begin(9600);
  // pinMode(1, FUNCTION_0); 
  pinMode(0, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

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
  // server.on("/1", handleOpen);
  // server.on("/0", handleClose);
  server.on("/motion", handleMotion);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
}

void loop(void)
{
  if (motion_trigger_counter > 0)
  {
    motion_trigger_counter--;
  }
  else
  {
    if (isMotionDetected == true)
    {
      turnOff();
      isMotionDetected = false;
    }
  }

  server.handleClient();

  uint8_t state = digitalRead(SENSOR);

  if (state > 0)
  {
    isMotionDetected = true;
    motion_trigger_counter = 30;
    turnOn();
  }

  delay(500);
}
