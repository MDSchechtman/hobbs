#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button2.h"
#include "FastLED.h"
#include <Encoder.h>
#include <Arduino.h>

// my stuff
#define RED 0
#define GREEN 1
#define BLUE 2
int current_color = RED; // 0 - R, 1 - G, 2 - B
int rgb[] = {0, 0, 0};
boolean color_selected = false;
int delta = 1;
//

#define NUM_LEDS 10
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 8
// #define CLK_PIN 4
#define VOLTS 5
#define MAX_MA 150

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

void reset() {
  color_selected = false;
  rgb[RED] = 0;
  rgb[GREEN] = 0;
  rgb[BLUE] = 0;
}

void loop_color(bool clockwise) {
  if (clockwise)
    {
      current_color++;
    }
    else
    {
      current_color--;
    }

    // loop around
    if (current_color > BLUE)
    {
      current_color = RED;
    }
    else if (current_color < RED)
    {
      current_color = BLUE;
    }

    // Serial.print(F("Current color: "));
    // Serial.println(current_color);
}

// Rotary Encoder
#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define ROTARY_BUTTON_PIN 13
Button2 rotary_button;
Encoder encoder(2, 3);

void handle_rotate(int position, int rpm, bool clockwise)
{
  if (!color_selected)
  {
    loop_color(clockwise);
  }
  else
  {
    if (clockwise)
    {
      rgb[current_color] += delta;
    }
    else
    {
      rgb[current_color] -= delta;
    }

    if (rgb[current_color] > 255)
    {
      rgb[current_color] = 255;
    }
    else if (rgb[current_color] < 0)
    {
      rgb[current_color] = 0;
    }

    // Serial.print(F("Current color value: "));
    // Serial.println(rgb[current_color]);
  }
}

// single click
void rotary_button_click(Button2 &btn)
{
  loop_color(true);
}

// long click
void rotary_button_long_click(Button2 &btn)
{
  color_selected = !color_selected;
}

void setup_encoder()
{
  rotary_button.begin(ROTARY_BUTTON_PIN);
  rotary_button.setClickHandler(rotary_button_click);
  rotary_button.setLongClickHandler(rotary_button_long_click);
}

#define MISC_BUTTON_PIN 10
Button2 misc_button;

void misc_button_click(Button2 &btn)
{
  if (delta != 10) {
    delta = 10;
  } else {
    delta = 1;
  }
}

void misc_button_long_click(Button2 &btn)
{
  reset();
}

void setup_misc_button()
{
  misc_button.begin(MISC_BUTTON_PIN, INPUT_PULLUP);
  misc_button.setClickHandler(misc_button_click);
  misc_button.setLongClickHandler(misc_button_long_click);
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Alive..."));

  delay(1000);

  setup_display();
  setup_led();
  setup_encoder();
  setup_misc_button();

  Serial.println(F("Setup complete."));
}

void draw_header()
{
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("(");
  display.print(rgb[RED]);
  display.print(", ");
  display.print(rgb[GREEN]);
  display.print(", ");
  display.print(rgb[BLUE]);
  display.print(")");

  display.print(F("  | "));
  display.println(delta);
}

void draw_body()
{
  display.setCursor(0, 20);

  if (!color_selected)
  {
    display.setTextSize(1);
    if (current_color == RED)
    {
      display.print(F("> "));
    }
    else
    {
      display.print(F("  "));
    }
    display.println(F("Red"));

    if (current_color == GREEN)
    {
      display.print(F("> "));
    }
    else
    {
      display.print(F("  "));
    }
    display.println(F("Green"));

    if (current_color == BLUE)
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
      display.println(rgb[RED]);
      break;
    case 1:
      display.print(F("Green: "));
      display.println(rgb[GREEN]);
      break;
    case 2:
      display.print(F("Blue: "));
      display.println(rgb[BLUE]);
      break;
    default:
      break;
    }
  }
}

int get_rpm(unsigned long current_time, unsigned long prev_time) {
  unsigned long timeBetweenLastPositions = current_time - prev_time;
  unsigned long timeToLastPosition = millis() - current_time;
  unsigned long t = max(timeBetweenLastPositions, timeToLastPosition);
  return 60000.0 / ((float)(t * 20));
}

long last_position  = -999;
unsigned long last_time = 0;
void loop()
{
  rotary_button.loop();
  misc_button.loop();

  long new_position = encoder.read();
  if (new_position != last_position) {
    int delta = new_position - last_position;
    last_position = new_position;

    long current_time = millis();
    int rpm = get_rpm(current_time, last_time);
    last_time = current_time;
    Serial.println(rpm);

    handle_rotate(last_position, rpm, delta > 0);
  }

  for (CRGB &pixel : leds)
  {
    pixel.red = rgb[RED];
    pixel.green = rgb[GREEN];
    pixel.blue = rgb[BLUE];
  }

  FastLED.show();

  // update display
  display.clearDisplay();
  draw_header();
  draw_body();
  display.display();
}