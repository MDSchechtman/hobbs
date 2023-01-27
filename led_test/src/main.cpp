#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button2.h"
#include "FastLED.h"
#include <Arduino.h>
#include <RotaryEncoder.h>

// my stuff
int current_color = 0; // 0 - R, 1 - G, 2 - B
int rgb[] = {0, 0, 0};
boolean color_selected = false;
//

#define NUM_LEDS 10
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 8
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
#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define ROTARY_BUTTON_PIN 13
Button2 rotary_button;
RotaryEncoder *encoder = nullptr;
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO_EVERY)
// This interrupt routine will be called on any change of one of the input signals
void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

#elif defined(ESP8266)
/**
 * @brief The interrupt service routine will be called on any change of one of the input signals.
 */
IRAM_ATTR void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

#endif

void handle_rotate(int position, int rpm, RotaryEncoder::Direction direction)
{
  if (!color_selected)
  {
    if (direction == RotaryEncoder::Direction::CLOCKWISE)
    {
      current_color++;
    }
    else if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE)
    {
      current_color--;
    }

    // loop around
    if (current_color > 2) {
      current_color = 0;
    } else if (current_color < 0) {
      current_color = 2;
    }
  }
  else 
  {
    int delta = 1;
    if (rpm > 150) {
      delta = 10;
    }

    if (direction == RotaryEncoder::Direction::CLOCKWISE)
    {
      rgb[current_color] += delta;
    }
    else if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE)
    {
      rgb[current_color] -= delta;
    }

    if (rgb[current_color] > 255) {
      rgb[current_color] = 0;
    } else if (rgb[current_color] < 0) {
      rgb[current_color] = 255;
    }
  }
}

// single click
void click(Button2 &btn)
{
  color_selected = !color_selected;
}

// long click
void resetPosition(Button2 &btn)
{
  color_selected = false;
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
}

void setup_encoder()
{
  encoder = new RotaryEncoder(ROTARY_PIN1, ROTARY_PIN2, RotaryEncoder::LatchMode::TWO03);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), checkPosition, CHANGE);

  rotary_button.begin(ROTARY_BUTTON_PIN);
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
  rotary_button.loop();

  static int pos = 0;
  encoder->tick(); // just call tick() to check the state.

  int newPos = encoder->getPosition();
  if (pos != newPos) {
    pos = newPos;
    int rpm = encoder->getRPM();
    Serial.println(rpm);
    handle_rotate(pos, rpm, encoder->getDirection());
  }

  for (CRGB &pixel : leds)
  {
    pixel.red = rgb[0];
    pixel.green = rgb[1];
    pixel.blue = rgb[2];
  }

  FastLED.show();

  // update display
  display.clearDisplay();
  draw_header();
  draw_body();
  display.display();
}