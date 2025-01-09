#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>

// Initialize WiFi connection
void initWifi(const char *ssid, const char *password)
{
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        M5.Display.print(".");
    }

    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.print(WiFi.localIP());
}
