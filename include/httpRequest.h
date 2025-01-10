// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "credentials.h"

void httpTestRequest()
{
    WiFiClientSecure *client = new WiFiClientSecure;

    if (!client)
    {
        M5.Display.println("[HTTPS] Failed to create client");
        return;
    }

    client->setCACert(awsRootCACertificate);
    client->setHandshakeTimeout(30); // Give more time for handshake

    HTTPClient https;

    M5.Display.println("[HTTPS] begin...");
    if (https.begin(*client, "https://test.hospital-policy-chat.com/"))
    {
        M5.Display.println("[HTTPS] GET...");
        int httpCode = https.GET();

        if (httpCode > 0)
        {
            M5.Display.printf("[HTTPS] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = https.getString();
                M5.Display.println(payload.substring(0, 100)); // First 100 chars
            }
        }
        else
        {
            M5.Display.printf("[HTTPS] GET failed: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
    }

    delete client;
}