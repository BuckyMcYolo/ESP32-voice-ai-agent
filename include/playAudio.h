#ifndef PLAYAUDIO_H
#define PLAYAUDIO_H

// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "credentials.h"

void playAudio(const char *url)
{
    M5.Display.println("[DEBUG] Starting playAudio function");

    WiFiClientSecure *client = new WiFiClientSecure;
    if (!client)
    {
        M5.Display.println("[ERROR] Failed to create client");
        return;
    }

    client->setCACert(awsRootCACertificate);
    client->setHandshakeTimeout(30);

    HTTPClient https;

    if (https.begin(*client, url))
    {
        M5.Display.println("[DEBUG] HTTPS connection established");
        int httpCode = https.GET();
        M5.Display.printf("[DEBUG] HTTP Code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            int len = https.getSize();
            M5.Display.printf("[DEBUG] File size: %d bytes\n", len);

            if (len > 0)
            {

                uint8_t *buffer = (uint8_t *)malloc(len);
                if (!buffer)
                {
                    M5.Display.println("[ERROR] Memory allocation failed");
                    https.end();
                    return;
                }

                WiFiClient *stream = https.getStreamPtr();
                int totalRead = 0;

                while (https.connected() && totalRead < len)
                {
                    size_t availableBytes = stream->available();
                    if (availableBytes > 0)
                    {
                        int bytesRead = stream->read(buffer + totalRead, availableBytes);
                        if (bytesRead < 0)
                            break;
                        totalRead += bytesRead;
                    }
                    delay(1);
                }
                M5.Display.clearDisplay();
                M5.Display.setCursor(0, 0);
                M5.Display.printf("[DEBUG] Download complete: %d bytes\n", totalRead);
                M5.Display.clearDisplay();
                M5.Display.setCursor(0, 0);

                if (totalRead == len)
                {
                    // Initialize speaker
                    M5.Speaker.begin();
                    M5.Speaker.setVolume(128);

                    M5.Display.println("[DEBUG] Starting playback");
                    bool result = M5.Speaker.playWav(buffer, totalRead);

                    while (M5.Speaker.isPlaying())
                    {
                        delay(100);
                    }
                }

                free(buffer);
            }
        }
        https.end();
    }
    delete client;
}
#endif