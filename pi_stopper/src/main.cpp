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
// must be created with secrets
#include "credentials.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary Encoder
#define ROTARY_ENCODER_A_PIN 27 // DT (double check this)
#define ROTARY_ENCODER_B_PIN 26 // CLK (double check this)
#define ROTARY_ENCODER_BUTTON_PIN 25 // button
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
int minutes = 0;

bool pihole_disabled = false;
int countdown = 0;
long last_interaction = 0;
bool dimmed = false;
int default_minutes = 1;

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void displayForMinutes(int m) {
  display.setCursor(0, 20);
  display.println("Pihole active...");
  display.println("");
  display.print("Disable for ");
  display.print(m);
  display.println(" mins");
}

void displayDisabled(int seconds) {
  display.setCursor(0, 20);
  display.println("Pihole DISABLED!!!");
  display.println("");
  display.println("Resuming in... ");
  display.print(seconds);
  display.println(" seconds");
}

String disablePiHole() {
  Serial.println("Attempting to disable pi hole");
  HTTPClient http;
  http.setReuse(false); 

  int seconds = minutes * 60;
  String part1 = "http://192.168.1.101/admin/api.php?disable=";
  String part2 = part1 + seconds;
  String part3 = part2 + "&auth=";
  String part4 = part3 + PIHOLE_TOKEN;
  String uri = part4;
  Serial.print("GET ");
  Serial.println(uri);

  http.begin(uri);

  // Send HTTP request
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
  http.setReuse(false); 

  String part1 = "http://192.168.1.101/admin/api.php?enable";
  String part2 = part1 + "&auth=" + PIHOLE_TOKEN;
  String uri = part2;
  Serial.print("GET ");
  Serial.println(uri);

  http.begin(uri);

  // Send HTTP request
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
    displayForMinutes(default_minutes);
    minutes = default_minutes;
  }
}

void connect_wifi() {

}

void setup() {
  Serial.begin(9600);

  // wifi
  WiFi.begin(WIFI_SSID, WIFI_PW);
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

  //display.setFont(&FreeSerif9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("Hello, world!");
  display.display(); 
  delay(2000);

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  rotaryEncoder.setBoundaries(1, 61, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  rotaryEncoder.setAcceleration(10);
  rotaryEncoder.setEncoderValue(5);

  displayForMinutes(rotaryEncoder.readEncoder());
}

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
        display.fillRect(102,7,4,1,WHITE);
        display.fillRect(107,6,4,2,WHITE);
        display.drawRect(112,4,4,4,WHITE);
        display.drawRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      } else if (rssi < -85 & rssi > -96) {
        display.fillRect(102,7,4,1,WHITE);
        display.drawRect(107,6,4,2,WHITE);
        display.drawRect(112,4,4,4,WHITE);
        display.drawRect(117,2,4,6,WHITE);
        display.drawRect(122,0,4,8,WHITE);
      } else {
        display.drawRect(102,7,4,1,WHITE);
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

    last_interaction = millis();
    display.dim(false);
    dimmed = false;
  }

  if (pihole_disabled && countdown > 0) {
    displayDisabled(countdown/1000);
    countdown -= 100;
  }

  if (pihole_disabled && countdown <= 0) {
    pihole_disabled = false;
    enablePiHole();
    displayForMinutes(default_minutes);
    minutes = default_minutes;
  }

  if (!pihole_disabled) {
    if (rotaryEncoder.encoderChanged()) {
      minutes = rotaryEncoder.readEncoder();
      last_interaction = millis();
      display.dim(false);
      dimmed = false;
    }
    displayForMinutes(minutes);
  }

  display.display();

  if (!dimmed && millis() - last_interaction > 5000) {
    display.dim(true);
    Serial.println("Dimming display");
    dimmed = true;
  }

  if (pihole_disabled) {
    delay(100);
  } else if (dimmed) {
    delay(250);
  }
}
