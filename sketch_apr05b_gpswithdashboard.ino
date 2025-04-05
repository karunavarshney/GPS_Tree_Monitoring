#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// GPS setup
static const int RXPin = D7, TXPin = D8;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// WiFi credentials
const char* ssid = "AndroidAP_5223";    // Replace with your WiFi name
const char* password = "karuna1234";    // Replace with your WiFi password

// ThingSpeak settings
unsigned long channelID = 290686; // Replace with your ThingSpeak Channel ID (number only)
const char* writeAPIKey = "BO6Z9T20592JDOO4"; // Replace with your Write API Key

WiFiClient client;

// Tree's original (planted) GPS location
double baseLat = 29.860000;
double baseLon = 77.899000;

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  ThingSpeak.begin(client);
  Serial.println("GPS and ThingSpeak initialized");
}

void loop() {
  while (ss.available() > 0) {
    gps.encode(ss.read());

    if (gps.location.isUpdated()) {
      double currentLat = gps.location.lat();
      double currentLon = gps.location.lng();

      double distance = TinyGPSPlus::distanceBetween(currentLat, currentLon, baseLat, baseLon);
      int status = (distance > 1000) ? 1 : 0;

      // Debug output
      Serial.print("Latitude: ");
      Serial.println(currentLat, 6);
      Serial.print("Longitude: ");
      Serial.println(currentLon, 6);
      Serial.print("Distance from base: ");
      Serial.print(distance, 2);
      Serial.println(" meters");
      Serial.print("Status: ");
      Serial.println(status == 1 ? "Moved" : "Safe");

      // Send to ThingSpeak
      ThingSpeak.setField(1, (float)currentLat);
      ThingSpeak.setField(2, (float)currentLon);
      ThingSpeak.setField(3, (float)distance);
      ThingSpeak.setField(4, status);

      int result = ThingSpeak.writeFields(channelID, writeAPIKey);
      if (result == 200) {
        Serial.println("Data sent to ThingSpeak!");
      } else {
        Serial.print("Failed to send data. Error code: ");
        Serial.println(result);
      }

      Serial.println("--------------------------");
      delay(15000); // ThingSpeak accepts updates every 15 sec
    }
  }
}
