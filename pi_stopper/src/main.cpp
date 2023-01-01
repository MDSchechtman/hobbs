#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AiEsp32RotaryEncoder.h"
#include <Arduino.h>
// web server
#include <WiFi.h>
#include <HTTPClient.h>

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
  display.println("Pihole active...");
  display.println("");
  display.print("Disable for ");
  display.print(m);
  display.println(" mins");
}

void displayDisabled(int seconds) {
  display.println("Pihole DISABLED!!!");
  display.println("");
  display.println("Resuming in... ");
  display.print(seconds);
  display.println(" seconds");
}

String disablePiHole() {
  HTTPClient http;

  // Your IP address with path or Domain name with URL path 
  String serverName = "http://192.168.1.101/admin/api.php?disable=" + minutes + String("&auth=cc82c777314048eb186cf0721cb0279b7862fd9303b33ac1cfad863a0e817247");
  http.begin(serverName.c_str());

  // Send HTTP POST request
  int httpResponseCode = http.GET();

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
  HTTPClient http;

  // Your IP address with path or Domain name with URL path 
  http.begin("http://192.168.1.101/admin/api.php?enable&auth=cc82c777314048eb186cf0721cb0279b7862fd9303b33ac1cfad863a0e817247");

  // Send HTTP POST request
  int httpResponseCode = http.GET();

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

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  if (rotaryEncoder.isEncoderButtonClicked()) {
    pihole_disabled = !pihole_disabled;
    rotary_onButtonClick();
    countdown = minutes * 60 * 1000; // 100ms ticks
  }

  if (pihole_disabled && countdown > 0) {
    displayDisabled(countdown/1000);
    countdown -= 100;
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
