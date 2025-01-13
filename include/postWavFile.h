#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "credentials.h"

static const char *SERVER_URL = sttEndpoint;

char *postWavData(const uint8_t *wavData, size_t wavSize)
{
    char *httpResult = NULL;

    if (!wavData || wavSize == 0)
    {
        httpResult = strdup("Invalid input data");
        M5.Display.println(httpResult);
        return httpResult;
    }

    WiFiClientSecure *client = new WiFiClientSecure;
    if (!client)
    {
        httpResult = strdup("Failed to create client");
        M5.Display.println(httpResult);
        return httpResult;
    }

    // Configure client
    client->setCACert(awsRootCACertificate);
    client->setHandshakeTimeout(5000);
    client->setTimeout(30000); // 30s timeout

    // Create HTTP client and begin connection
    HTTPClient http;
    if (!http.begin(*client, SERVER_URL))
    {
        httpResult = strdup("Failed to begin HTTP connection");
        M5.Display.println(httpResult);
        delete client;
        return httpResult;
    }

    http.addHeader("Content-Type", "audio/wav");
    int httpResponseCode = http.sendRequest("POST", (uint8_t *)wavData, wavSize);

    M5.Display.printf("HTTP response code: %d\n", httpResponseCode);

    if (httpResponseCode > 0)
    {
        Serial.printf("POST... code: %d\n", httpResponseCode);

        if (httpResponseCode == HTTP_CODE_OK)
        {
            String response = http.getString();
            httpResult = strdup(response.c_str());

            if (!httpResult)
            {
                httpResult = strdup("Memory allocation failed for response");
            }

            Serial.println(response);
            M5.Display.println("Response:");
            M5.Display.println(httpResult ? httpResult : "Memory error");
        }
        else
        {
            String statusMsg = "Unexpected status code: " + String(httpResponseCode);
            httpResult = strdup(statusMsg.c_str());
            M5.Display.println(httpResult ? httpResult : "Memory error");
        }
    }
    else
    {
        String errorStr = "HTTP error: " + http.errorToString(httpResponseCode) + " (" + String(httpResponseCode) + ")";
        httpResult = strdup(errorStr.c_str());
        Serial.println(errorStr);
        M5.Display.println(httpResult ? httpResult : "Memory error");
    }

    if (!httpResult)
    {
        httpResult = strdup("Critical error: All allocations failed");
    }

    // Clean up
    http.end();
    delete client;

    return httpResult;
}