#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "credentials.h"

static const char *SERVER_URL = "YOUR_API_ENDPOINT";

void postWavData(const uint8_t *wavData, size_t wavSize)
{
    WiFiClientSecure client;
    client.setInsecure(); // or use root certificate

    HTTPClient http;
    if (http.begin(client, SERVER_URL))
    {
        // Example: if your endpoint expects "audio/wav"
        http.addHeader("Content-Type", "audio/wav");

        // POST the entire WAV in one go
        int httpResponseCode = http.sendRequest("POST", (uint8_t *)wavData, wavSize);

        if (httpResponseCode > 0)
        {
            Serial.printf("POST... code: %d\n", httpResponseCode);
            if (httpResponseCode == HTTP_CODE_OK)
            {
                String response = http.getString();
                Serial.println(response);
                M5.Display.println("Response:");
                M5.Display.println(response);
            }
        }
        else
        {
            Serial.printf("POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
            M5.Display.println("POST error!");
        }
        http.end();
    }
    else
    {
        M5.Display.println("HTTP connection failed!");
    }
}