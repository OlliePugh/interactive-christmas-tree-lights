#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "Secrets.h"

#define LED_PIN 26
#define NUM_LEDS 50

CRGB leds[NUM_LEDS];

// SSID & Password
const char *ssid = WIFI_SSID;     // Enter your SSID here
const char *password = WIFI_PASS; // Enter your Password here

AsyncWebServer server(80); // Object of WebServer(HTTP port, 80 is defult)

// Handle root url (/)
void handle_root(AsyncWebServerRequest *request)
{
  if (!request->hasHeader("Authorization"))
  {
    request->send(401);
    return;
  }

  AsyncWebHeader *h = request->getHeader("Authorization");
  if (h->value() != API_KEY) // does it have the api key
  {
    request->send(401);
    return;
  }

  if (!request->hasParam("colour") || !request->hasParam("bulb-id")) // does it have the required params
  {
    request->send(400);
    return;
  }

  AsyncWebParameter *colour = request->getParam("colour");
  AsyncWebParameter *bulbId = request->getParam("bulb-id");
  int bulbIdInt = bulbId->value().toInt();
  int colourHex = (int)strtol(colour->value().c_str(), NULL, 16);
  Serial.printf("Updating light %d to colour %s\n", bulbIdInt, colour->value());

  // Split them up into r, g, b values
  int g = colourHex >> 16;
  int r = colourHex >> 8 & 0xFF;
  int b = colourHex & 0xFF;
  leds[bulbIdInt] = CRGB(r, g, b);
  request->send(200);
}

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  for (size_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(255, 255, 255);
  }
  FastLED.show();
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);

  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); // Show ESP32 IP on serial

  server.on("/", HTTP_GET, handle_root);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop()
{
  FastLED.show(); // update the lights
}

// void setup()
// {
//   FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
//   for (size_t i = 0; i < NUM_LEDS; i++)
//   {
//     leds[i] = CRGB(random(30), random(30), random(30));
//   }
//   FastLED.show();
// }

// void loop()
// {
//   int randomLed = random(49);
//   leds[randomLed] = CRGB(random(30), random(30), random(30));
//   FastLED.show();
//   delay(random(500, 1500));
// }
