/**
 * Receive RGB data using Glediator protocol and output to multiple strips.
 * 
 * FastLED-Glediator code adapted from: https://github.com/RanzQ/serial-fastled/blob/master/GlediatorFastled.ino
 */
#include <FastLED.h>

#define NUM_STRIPS 3 // you also need to edit the setup method below
#define NUM_LEDS_PER_STRIP 9

#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define CMD_NEW_DATA 1
#define BAUD_RATE 1000000

CRGB leds[NUM_LEDS];

void setup() {
  // add/remove line if you have more/less than 3 strips (don't forget to edit NUM_STRIPS above as well)
  //              <Type> <PIN><Order>         <OFFSET>              <LENGTH>
  FastLED.addLeds<WS2812B, 3, GRB>(leds, NUM_LEDS_PER_STRIP*0, NUM_LEDS_PER_STRIP); // offset = 0
  FastLED.addLeds<WS2812B, 5, GRB>(leds, NUM_LEDS_PER_STRIP*1, NUM_LEDS_PER_STRIP); // offset = NUM_LEDS_PER_STRIP
  FastLED.addLeds<WS2812B, 6, GRB>(leds, NUM_LEDS_PER_STRIP*2, NUM_LEDS_PER_STRIP); // offset = 2 * NUM_LEDS_PER_STRIP
  //...
  //FastLED.addLeds<WS2812B, 9, GRB>(leds, NUM_LEDS_PER_STRIP*3, NUM_LEDS_PER_STRIP); // Pin 9
  //FastLED.addLeds<WS2812B, 10, GRB>(leds, NUM_LEDS_PER_STRIP*4, NUM_LEDS_PER_STRIP);// Pin 10
  //FastLED.addLeds<WS2812B, 11, GRB>(leds, NUM_LEDS_PER_STRIP*5, NUM_LEDS_PER_STRIP);// Pin 11
  
  Serial.begin(BAUD_RATE);
}

void loop() {
  while (serialGlediator () != CMD_NEW_DATA) {}
    Serial.readBytes((char*)leds, NUM_LEDS*3);
    FastLED.show();
}

int serialGlediator () {
  while (!Serial.available()) {}
  return Serial.read();
}
