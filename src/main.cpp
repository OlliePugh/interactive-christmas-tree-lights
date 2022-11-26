#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Secrets.h"

#define LED_PIN 26
#define NUM_LEDS 50

CRGB leds[NUM_LEDS];

// SSID & Password
const char *ssid = WIFI_SSID;     // Enter your SSID here
const char *password = WIFI_PASS; // Enter your Password here
const char *getFullLightsUrl = GET_FULL_LIGHTS_URL;

AsyncWebServer server(80); // Object of WebServer(HTTP port, 80 is defult)
float intensityReducer = 0.2;

void setLightColour(int index, const String *hex)
{
  int colourHex = (int)strtol(hex->c_str(), NULL, 16);
  Serial.printf("Updating light %d to colour %s\n", index, hex);

  // Split them up into r, g, b values
  int g = colourHex >> 16;
  int r = colourHex >> 8 & 0xFF;
  int b = colourHex & 0xFF;
  leds[index] = CRGB(r * intensityReducer, g * intensityReducer, b * intensityReducer);
}

void initialiseLights()
{
  Serial.println("Getting full lights status");
  HTTPClient http;
  Serial.println(getFullLightsUrl);
  http.begin(getFullLightsUrl);
  http.addHeader("Authorization", API_KEY);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    JsonArray arr = doc.as<JsonArray>();

    int counter = 0;
    for (JsonVariant value : arr)
    {
      String hex = value.as<String>();
      hex.remove(0, 1); // remove the #
      setLightColour(counter++, &hex);
    }
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

// Handle root url (/)
void handleRoot(AsyncWebServerRequest *request)
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
  setLightColour(bulbIdInt, &colour->value());
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

  initialiseLights();

  server.on("/", HTTP_GET, handleRoot);

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
