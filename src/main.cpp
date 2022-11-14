#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 26
#define NUM_LEDS 50

CRGB leds[NUM_LEDS];

void setup()
{
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop()
{
  CRGB randomColour1 = CRGB(random(255), random(255), random(255));
  CRGB randomColour2 = CRGB(random(255), random(255), random(255));
  for (size_t i = 0; i < NUM_LEDS; i++)
  {
    if (i % 2 == 0)
    {
      leds[i] = randomColour1;
    }
    else
    {
      leds[i] = randomColour2;
    }
    delay(10);
    FastLED.show();
  }
}