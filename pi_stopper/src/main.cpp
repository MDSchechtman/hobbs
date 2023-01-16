#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AiEsp32RotaryEncoder.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary Encoder
#define ROTARY_ENCODER_A_PIN 27
#define ROTARY_ENCODER_B_PIN 26
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
int minutes = 0;

// web server
const char* ssid = "starknet";
const char* password = "starknet";

bool pihole_disabled = false;
int countdown = 0;

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void displayForMinutes(int m) {
  display.setCursor(0, 10);
  display.println("Pihole active...");
  display.println("");
  display.print("Disable for ");
  display.print(m);
  display.println(" mins");
}

void displayDisabled(int seconds) {
  display.setCursor(0, 10);
  display.println("Pihole DISABLED!!!");
  display.println("");
  display.println("Resuming in... ");
  display.print(seconds);
  display.println(" seconds");
}

String disablePiHole() {
  Serial.println("Attempting to disable pi hole");
  HTTPClient http;

  String uri = "http://192.168.1.101/admin/api.php?disable=" + minutes + String("&auth=<YOUR KEY>");
  Serial.print("GET ");
  Serial.println(uri);

  http.begin(uri);
  int httpResponseCode = http.GET();

  // not using
  String payload = "{}"; 

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}

String enablePiHole() {
  Serial.println("Attempting to enable pi hole");
  HTTPClient http;

  String uri = "http://192.168.1.101/admin/api.php?enable&auth=<YOUR KEY>";
  Serial.print("GET ");
  Serial.println(uri);

  http.begin(uri);
  int httpResponseCode = http.GET();

  // not using
  String payload = "{}"; 

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}

void rotary_onButtonClick()
{
    static unsigned long lastTimePressed = 0;
    if (millis() - lastTimePressed < 200)
        return;
    lastTimePressed = millis();

  if (pihole_disabled) {
    disablePiHole();
  } else {
    enablePiHole();
    displayForMinutes(5);
    minutes = 5;
  }
}

void setup() {
  Serial.begin(9600);

  // wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // sda, scl
  Wire.begin();

	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.println(F("Display initialized!"));
  display.clearDisplay();

  display.setFont(&FreeSerif9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("Hello, world!");
  display.display(); 
  delay(2000);

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(0, 61, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(10);
  rotaryEncoder.setEncoderValue(5);

  displayForMinutes(rotaryEncoder.readEncoder());
}

// not using a battery so just draw the nice number
// Put something else in this corner? idk.
void draw_battery() {
  display.setCursor(0, 0);
  display.print("69%");
}

wl_status_t get_and_draw_wifi_status() {
  long rssi = WiFi.RSSI();
  // Serial.print("RSSI: ");
  // Serial.print(rssi);

  wl_status_t status = WiFi.status();
  // Serial.print(" WiFi Status: ");
  // Serial.print(status);
  // Serial.println();
  
  // draw the wifi bars
  switch(WiFi.status()) {
    case WL_CONNECTED:
      if (rssi >= -55) { 
        display.fillRect(102,7,4,1,WHITE);
        display.fillRect(107,6,4,2,WHITE);
        display.fillRect(112,4,4,4,WHITE);
        display.fillRect(117,2,4,6,WHITE);
        display.fillRect(122,0,4,8,WHITE);
      } else if (rssi < -55 & rssi > -65) {
        display.fillRect(102,7,4,1,WHITE);
        display.fillRect(107,6,4,2,WHITE);
        display.fillRect(112,4,4,4,WHITE);
        display.fillRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      } else if (rssi < -75 & rssi > -85) {
        display.fillRect(102,8,4,1,WHITE);
        display.fillRect(107,6,4,2,WHITE);
        display.drawRect(112,4,4,4,WHITE);
        display.drawRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      } else if (rssi < -85 & rssi > -96) {
        display.fillRect(102,8,4,1,WHITE);
        display.drawRect(107,6,4,2,WHITE);
        display.drawRect(112,4,4,4,WHITE);
        display.drawRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      } else {
        display.drawRect(102,8,4,1,WHITE);
        display.drawRect(107,6,4,2,WHITE);
        display.drawRect(112,4,4,4,WHITE);
        display.drawRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      }
      break;
    case WL_DISCONNECTED:
    case WL_CONNECTION_LOST:
      display.setCursor(120, 0);
      display.print("D/C");
      break;
    case WL_IDLE_STATUS:
      display.setCursor(120, 0);
      display.print("IDLE");
      break;
    case WL_NO_SSID_AVAIL:
      display.setCursor(120, 0);
      display.print("SSID");
      break;
    default:
      display.setCursor(120, 0);
      display.print("OOPS");
      break;
  }

  return status;
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  draw_battery();
  wl_status_t wifiStatus = get_and_draw_wifi_status();
  if (wifiStatus != WL_CONNECTED) {
    Serial.println("Attempting to reconnect...");
    WiFi.reconnect();
  }
  
  if (rotaryEncoder.isEncoderButtonClicked()) {
    Serial.println("Button clicked!");
    pihole_disabled = !pihole_disabled;
    rotary_onButtonClick();
    countdown = minutes * 60 * 1000; 
  }

  if (pihole_disabled && countdown > 0) {
    displayDisabled(countdown/1000);
    countdown -= 100;
    // not doing anything so slow down to 100ms between loops
    delay(100);
  }

  if (countdown < 0) {
    pihole_disabled = false;
  }

  if (!pihole_disabled) {
    if (rotaryEncoder.encoderChanged()) {
      minutes = rotaryEncoder.readEncoder();
    }
    displayForMinutes(minutes);
  }

  display.display();
}
