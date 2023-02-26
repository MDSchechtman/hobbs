#include <SPI.h>
#include <WiFi.h>
#include <Arduino.h>

#define STA_SSID "starknet"
#define STA_PASS "starknet"

int status = WL_IDLE_STATUS;

WiFiServer server(80);

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(9600);

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(STA_SSID);
    status = WiFi.begin(STA_SSID, STA_PASS);
    delay(10000);
  }

  server.begin();
  printWifiStatus();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        // if you've gotten to the end of the line (received a newline

        // character) and the line is blank, the http request has ended,

        // so you can send a reply

        if (c == '\n' && currentLineIsBlank) {

          // send a standard http response header

          client.println("HTTP/1.1 200 OK");

          client.println("Content-Type: text/html");

          client.println("Connection: close");  // the connection will be closed after completion of the response

          client.println("Refresh: 5");  // refresh the page automatically every 5 sec

          client.println();

          client.println("<!DOCTYPE HTML>");

          client.println("<html>");

          // output the value of each analog input pin

          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {

            int sensorReading = analogRead(analogChannel);

            client.print("analog input ");

            client.print(analogChannel);

            client.print(" is ");

            client.print(sensorReading);

            client.println("<br />");

          }

          client.println("</html>");

          break;

        }

        if (c == '\n') {

          // you're starting a new line

          currentLineIsBlank = true;

        } else if (c != '\r') {

          // you've gotten a character on the current line

          currentLineIsBlank = false;

        }

      }

    }

    // give the web browser time to receive the data

    delay(1);

    // close the connection:

    client.stop();

    Serial.println("client disconnected");
  }
}