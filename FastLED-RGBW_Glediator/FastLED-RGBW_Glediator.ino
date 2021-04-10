/* Glediator protocol for RGBW strips (SK6812) using FastLED_RGBW hack and
 *  RGB to RGBW transformation.
 *  
 * Original code by Daniel Murphy (https://gist.github.com/dmurph/c650904699d0b3a6db3dfefd8c2fded4)
 * -> Blog post: https://www.dmurph.com/posts/2021/1/cabinet-light-3.html
 * 
 * See FastLED_RGBW.h for credits.
 * 
 * Modified for use with the Glediator protocol by Drumber (https://github.com/Drumber)
 */

#include "FastLED.h"
#include "FastLED_RGBW.h"

#define NUM_LEDS 60
#define DATA_PIN 6
#define CMD_NEW_DATA 1
#define BAUD_RATE 1000000

CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

const uint8_t brightness = 200;

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.setBrightness(brightness);
  FastLED.show();

  delay(10);
  Serial.begin(BAUD_RATE);
}

void loop() {
  while (serialGlediator () != CMD_NEW_DATA) {}
  
  uint8_t tmp[NUM_LEDS*3];
  Serial.readBytes(tmp, NUM_LEDS*3);
    
  for(int i = 0; i < NUM_LEDS; i++) {
    int pos = i*3; // data offset for current pixel i
    CRGB rgb(tmp[pos], tmp[pos+1], tmp[pos+2]);

    //-- 1. RGB transformation with simple calculation
    leds[i] = GetRgbwFromRgb1(rgb);
    
    //-- 2. RGBW transformation with color temperature
    //leds[i] = GetRgbwFromRgb2(rgb);
    
    //-- OR: use this instead if you don't want to use the white channel (W = 0)
    //leds[i] = CRGBW(rgb.r, rgb.g, rgb.b, 0);
  }
  
  FastLED.show();
}

int serialGlediator () {
  while (!Serial.available()) {}
  return Serial.read();
}


/*
 * Simple RGB to RGBW Transformation by subtracting the smallest RGB
 * value from all RGB value and setting W to it.
 * 
 * + fast, does not need much calculation
 * - does not respect color temperature
 */

// Kind of from https://stackoverflow.com/questions/40312216/converting-rgb-to-rgbw
// Simlified to not include all the hsb stuff by just taking the
// minimum value of the components and making that white, and then reducing
// the rgb components by that value.
CRGBW GetRgbwFromRgb1(CRGB rgb) {
  //Get the maximum between R, G, and B
  uint8_t Ri = rgb.r;
  uint8_t Gi = rgb.g;
  uint8_t Bi = rgb.b;
  uint8_t minVal = min(Ri, min(Gi, Bi));
  
  uint8_t Wo = minVal;
  uint8_t Bo = Bi - minVal;
  uint8_t Ro = Ri - minVal;
  uint8_t Go = Gi - minVal;

  return CRGBW(Ro, Go, Bo, Wo);
}


/*
 * Advanced RGB to RGBW Transformation that respects the color
 * temperature of the white LED.
 * 
 * + true white color
 * + supports different white temperatures (WW, NC, CC)
 * - slower processing, flickering/lagging when using Glediator protocol
 *   with many pixels
 */

// Reference, currently set to 4500k white light:
// https://andi-siess.de/rgb-to-color-temperature/
const uint8_t kWhiteRedChannel = 255;
const uint8_t kWhiteGreenChannel = 219;
const uint8_t kWhiteBlueChannel = 186;

// The transformation has to be normalized to 255
static_assert(kWhiteRedChannel >= 255 || kWhiteGreenChannel >= 255 || kWhiteBlueChannel >= 255);

CRGBW GetRgbwFromRgb2(CRGB rgb) {
  //Get the maximum between R, G, and B
  uint8_t r = rgb.r;
  uint8_t g = rgb.g;
  uint8_t b = rgb.b;

  // These values are what the 'white' value would need to2
  // be to get the corresponding color value.
  double whiteValueForRed = r * 255.0 / kWhiteRedChannel;
  double whiteValueForGreen = g * 255.0 / kWhiteGreenChannel;
  double whiteValueForBlue = b * 255.0 / kWhiteBlueChannel;

  // Set the white value to the highest it can be for the given color
  // (without over saturating any channel - thus the minimum of them).
  double minWhiteValue = min(whiteValueForRed, min(whiteValueForGreen, whiteValueForBlue));
  uint8_t Wo = (minWhiteValue <= 255 ? (uint8_t) minWhiteValue : 255);

  // The rest of the channels will just be the origina value minus the
  // contribution by the white channel.
  uint8_t Ro = (uint8_t)(r - minWhiteValue * kWhiteRedChannel / 255);
  uint8_t Go = (uint8_t)(g - minWhiteValue * kWhiteGreenChannel / 255);
  uint8_t Bo = (uint8_t)(b - minWhiteValue * kWhiteBlueChannel / 255);

  return CRGBW(Ro, Go, Bo, Wo);
}
