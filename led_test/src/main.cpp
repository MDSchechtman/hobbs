#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button2.h"
#include "ESPRotary.h"
#include "FastLED.h"
#include <Arduino.h>

// my stuff
int current_color = 0; // 0 - R, 1 - G, 2 - B
int rgb[] = {0, 0, 0};
boolean color_selected = false;
//

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
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true))
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
#define ROTARY_PIN1 11
#define ROTARY_PIN2 12
#define BUTTON_PIN 13
#define CLICKS_PER_STEP 2
ESPRotary rotary;
Button2 rotary_button;

void rotate(ESPRotary &r)
{
  if (!color_selected)
  {
    if (r.getDirection() == right)
    {
      current_color++;
    }
    else
    {
      current_color--;
    }

    // loop around
    current_color = current_color % 3; 
  }
  else 
  {
    if (r.getDirection() == right)
    {
      rgb[current_color]++;
    }
    else
    {
      rgb[current_color]--;
    }
  }
  Serial.println(r.getPosition());
}

// on left or right rotation
void showDirection(ESPRotary &r)
{
  Serial.println(r.directionToString(r.getDirection()));
}

// single click
void click(Button2 &btn)
{
  color_selected = !color_selected;
  Serial.println(F("Click!"));
}

// long click
void resetPosition(Button2 &btn)
{
  rotary.resetPosition();
  color_selected = false;
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
  Serial.println(F("Reset!"));
}

void setup_encoder()
{
  rotary.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, 0, 255, 0, 1);
  rotary.enableSpeedup(true);
  rotary.setChangedHandler(rotate);
  //rotary.setLeftRotationHandler(showDirection);
  //rotary.setRightRotationHandler(showDirection);

  rotary_button.begin(BUTTON_PIN);
  rotary_button.setTapHandler(click);
  rotary_button.setLongClickHandler(resetPosition);
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Alive..."));

  delay(1000);

  setup_display();
  setup_led();
  setup_encoder();

  Serial.println(F("Setup complete."));
}

void draw_header()
{
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("(");
  display.print(rgb[0]);
  display.print(", ");
  display.print(rgb[1]);
  display.print(", ");
  display.print(rgb[2]);
  display.println(")");  
}

void draw_body()
{
  display.setCursor(0, 20);

  if (!color_selected)
  {
    display.setTextSize(1);
    if (current_color == 0)
    {
      display.print(F("> "));
    }
    else
    {
      display.print(F("  "));
    }
    display.println(F("Red"));

    if (current_color == 1)
    {
      display.print(F("> "));
    }
    else
    {
      display.print(F("  "));
    }
    display.println(F("Green"));

    if (current_color == 2)
    {
      display.print(F("> "));
    }
    else
    {
      display.print("  ");
    }
    display.println(F("Blue"));
  }
  else
  {
    display.setTextSize(2);
    switch (current_color)
    {
    case 0:
      display.print(F("Red: "));
      display.println(rgb[0]);
      break;
    case 1:
      display.print(F("Green: "));
      display.println(rgb[1]);
      break;
    case 2:
      display.print(F("Blue: "));
      display.println(rgb[2]);
      break;
    default:
      break;
    }
  }
}

void loop()
{
  rotary.loop();
  rotary_button.loop();

  for (CRGB &pixel : leds)
  {
    pixel.red = rgb[0];
    pixel.blue = rgb[1];
    pixel.green = rgb[2];
  }

  FastLED.show();

  // update display
  display.clearDisplay();
  draw_header();
  draw_body();
  display.display();
}