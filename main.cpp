#include <stdio.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "never.h"

// Hardware settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Declare global OLED and BME280 objects (hardware interfaces)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BME280 bme;

// Status flags and timing
bool sensorReady = false;
bool oledReady = false;
unsigned long lastUpdate = 0;
const long UPDATE_INTERVAL = 2000; 

// Create a web server object that listens on port 80 (standard HTTP port)
WebServer server(80);

// Generates the HTML webpage with sensor values (temperature & humidity)
String maakWebpagina(float temperatuur, float vochtigheid) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>ESP32 Sensor</title>";
  html += "</head><body>";
  // Add table with sensor values here
  html += "</body></html>";
  return html;
}

void handleRoot() {
    // Reads the temperature and humidity values
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    
    // Check if sensor returned invalid data (NaN = Not a Number)
    if (isnan(temp) || isnan(hum)) {
        // Show error page and stop
        server.send(200, "text/html", "<html><body>Sensor not available</body></html>");
        return;
    }
    
    // Sends text to the HTML page
    String sensorDisplayText = 
    "<pre style='font-family: monospace; margin:0;'>"
    "Temperature:  " + String(temp, 2) + " &deg;C\n"
    "Humidity:     " + String(hum, 2) + " %"
    "</pre>";

    // Here you build your HTML page with the instruction to refresh the page every 2 seconds
    String html = "<html><body>";
    html += sensorDisplayText;
    html += "<script>";
    html += " setTimeout(function() {";
    html += "location.reload();";
    html += "}, 2000);";
    html += "</script></body></html>";
    
    // Send the HTML webpage to the browser (HTTP 200 = OK)
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Start WiFi Access point
    WiFi.softAP("ESP32WiFi"); 
    
    // instructions for connecting MyESP32-WiFi on your phone
    Serial.println("Webserver started");
    Serial.println("ESP32 is ready for use");
    Serial.println("Go on your telephone:");
    Serial.println("1. Go to wiFi settings");
    Serial.println("2. Connect to WiFi > MyESP32");
    Serial.println("3. If connected tab on the right side settings");
    Serial.println("4. Go to manage router");
    Serial.print("5. It opens browser > http://");

    server.on("/", handleRoot); // link the URL (main page) to the handleRoot function
    server.begin(); // Start webServer

    Serial.println(WiFi.softAPIP()); // Give me the IP address of my own network
    Serial.println("See live sensor values!");

    // Initialize BME280 sensor
    if(bme.begin(0x76)) {
        sensorReady = true;
    } else {
        Serial.print("Sensor not found!");
    }
    
    // Initialize OLED screen
    if(display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
        oledReady = true;
    } else {
        Serial.print("Oled not found!");
    }
}

void loop() {
    // Checks if it's time for an update
     if(millis() - lastUpdate >= UPDATE_INTERVAL) { 
        if (sensorReady && oledReady) {
         float temp = bme.readTemperature();
         float hum = bme.readHumidity();
         
         // Checks for incorrect values
         if (!isnan(temp) && !isnan(hum)) {
            
            // Print sensor readings to Serial Monitor for debugging
            Serial.println("Temperature: " + String(temp, 2) + " °C");
            Serial.println("Humidity:    " + String(hum, 2) + " %");
   
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            
            display.println("Measure Enviroment");

            // Print sensor readings to Serial Monitor for debugging
            display.println("Temperature: " + String(temp, 2) + " C");
            display.println("Humidity:    " + String(hum, 2) + " %");

            display.display(); // display.update() is here 

    } else {
        sensorReady = false; // sensor fails > turn off
    }
  } 

    // remember when we did the last update
    lastUpdate = millis();
  }

  // Web server must always listen
  server.handleClient();
}
