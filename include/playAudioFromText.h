// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "credentials.h"

#include "playAudio.h"

void playAudioFromText(const char *text)
{
    WiFiClientSecure *client = new WiFiClientSecure;

    if (!client)
    {
        M5.Display.println("[HTTPS] Failed to create client");
        return;
    }

    client->setCACert(awsRootCACertificate);
    client->setHandshakeTimeout(30);

    HTTPClient https;

    // Step 1: POST text and get URL
    M5.Display.println("[HTTPS] Posting text...");
    if (https.begin(*client, ttsEndpoint))
    {
        String requestBody = String("{\"text\":\"") + text + "\"}";
        https.addHeader("Content-Type", "application/json");

        int httpCode = https.POST(requestBody);

        if (httpCode > 0)
        {
            String response = https.getString();

            // Parse JSON response to get URL
            // You might want to use ArduinoJson library for more robust parsing
            int urlStart = response.indexOf("\"url\":\"") + 7;
            int urlEnd = response.indexOf("\"", urlStart);
            String audioUrl = response.substring(urlStart, urlEnd);
            // M5.Display.println("Audio URL: " + audioUrl);

            // Clean up first request
            https.end();

            // Step 2: GET audio file from URL
            M5.Display.println("[HTTPS] Getting audio...");
            playAudio(audioUrl.c_str());
            // if (https.begin(*client, audioUrl))
            // {
            //     httpCode = https.GET();

            //     if (httpCode > 0)
            //     {
            //         int len = https.getSize();
            //         if (len > 0)
            //         {
            //             uint8_t *buffer = (uint8_t *)malloc(len);
            //             if (!buffer)
            //             {
            //                 M5.Display.println("Failed to allocate memory for WAV file");
            //                 https.end();
            //                 return;
            //             }

            //             WiFiClient *stream = https.getStreamPtr();
            //             int totalRead = 0;
            //             while (https.connected() && totalRead < len)
            //             {
            //                 size_t availableBytes = stream->available();
            //                 if (availableBytes > 0)
            //                 {
            //                     int bytesRead = stream->read(buffer + totalRead, availableBytes);
            //                     if (bytesRead < 0)
            //                         break;
            //                     totalRead += bytesRead;
            //                 }
            //                 delay(1);
            //             }

            //             M5.Speaker.playWav(buffer, totalRead);
            //             free(buffer);
            //         }
            //     }
            //     else
            //     {
            //         M5.Display.printf("Audio GET failed, code: %d\n", httpCode);
            //     }
            // }
        }
        else
        {
            M5.Display.printf("POST failed, code: %d\n", httpCode);
        }
        https.end();
    }
    else
    {
        M5.Display.println("HTTPS connection failed.");
    }

    delete client;
}