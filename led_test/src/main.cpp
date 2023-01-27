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
#define ROTARY_PIN1 11
#define ROTARY_PIN2 12
#define BUTTON_PIN 13
#define CLICKS_PER_STEP 4
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
    
    // loop around
    rgb[current_color] == rgb[current_color] % 255;
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
  Serial.println("Click!");
}

// long click
void resetPosition(Button2 &btn)
{
  rotary.resetPosition();
  color_selected = false;
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
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

void draw_header()
{
  display.setCursor(0, 0);

  String rgb_text0 = "(";
  String rgb_text1 = rgb_text0 + rgb[0];
  String rgb_text2 = rgb_text1 + ", ";
  String rgb_text3 = rgb_text2 + rgb[1];
  String rgb_text4 = rgb_text3 + ", ";
  String rgb_text5 = rgb_text4 + rgb[2];
  String rgb_text6 = rgb_text5 + ")";
  display.println(rgb_text6);
}

void draw_body()
{
  display.setCursor(20, 0);

  if (!color_selected)
  {
    if (current_color == 0)
    {
      display.print("> ");
    }
    else
    {
      display.print("  ");
    }
    display.println("Red");

    if (current_color == 1)
    {
      display.print("> ");
    }
    else
    {
      display.print("  ");
    }
    display.println("Green");

    if (current_color == 2)
    {
      display.print("> ");
    }
    else
    {
      display.print("  ");
    }
    display.println("Blue");
  }
  else
  {
    display.setTextSize(2);
    switch (current_color)
    {
    case 0:
      display.print("Red: ");
      display.println(rgb[0]);
      break;
    case 1:
      display.print("Green: ");
      display.println(rgb[1]);
      break;
    case 2:
      display.print("Blue: ");
      display.println(rgb[2]);
      break;
    default:
      break;
    }
  }
}

void loop()
{
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