#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button2.h"
#include "ESPRotary.h"
#include "FastLED.h"
#include <Arduino.h>

#define NUM_LEDS 10
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 3
// #define CLK_PIN 4
#define VOLTS 5
#define MAX_MA 1500

CRGBArray<NUM_LEDS> leds;
CRGB gBackgroundColor = CRGB::Black;

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

void setup_led()
{
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_MA);
  FastLED
      .addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
}

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup_display()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  Serial.println(F("Display initialized!"));
  display.clearDisplay();

  // display.setFont(&FreeSerif9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("Hello, world!");
  display.display();
}

// Rotary Encoder
#define ROTARY_PIN1	11
#define ROTARY_PIN2	12
#define BUTTON_PIN	13
#define CLICKS_PER_STEP 4   
ESPRotary rotary;
Button2 rotary_button;

void rotate(ESPRotary& r) {
   Serial.println(r.getPosition());
}

// on left or right rotation
void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
}

// single click
void click(Button2& btn) {
  Serial.println("Click!");
}

// long click
void resetPosition(Button2& btn) {
  rotary.resetPosition();
  Serial.println("Reset!");
}

void setup_encoder()
{
  rotary.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  rotary.setChangedHandler(rotate);
  rotary.setLeftRotationHandler(showDirection);
  rotary.setRightRotationHandler(showDirection);

  rotary_button.begin(BUTTON_PIN);
  rotary_button.setTapHandler(click);
  rotary_button.setLongClickHandler(resetPosition);
}

void setup()
{
  delay(3000);
  Serial.begin(115200);

  setup_led();
  setup_display();
  setup_encoder();
}

void loop()
{

  for (CRGB &pixel : leds)
  {
    pixel.red++;
    pixel.blue++;
    pixel.green++;
  }

  FastLED.show();
}