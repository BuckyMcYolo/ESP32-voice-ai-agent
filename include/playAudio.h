// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "credentials.h"

// void playAudio(const char *url)
void playAudio()

{

    WiFiClientSecure *client = new WiFiClientSecure;

    if (!client)
    {
        M5.Display.println("[HTTPS] Failed to create client");
        return;
    }
    // import root certificate

    client->setCACert(googleCloudStorageRootCa);
    client->setHandshakeTimeout(30); // Give more time for handshake

    HTTPClient https;

    M5.Display.println("[HTTPS] begin...");
    if (https.begin(*client, wavFileUrl))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // Get size of the wav file
            int len = https.getSize();
            if (len > 0)
            {
                // Allocate a buffer to hold the entire file
                uint8_t *buffer = (uint8_t *)malloc(len);
                if (!buffer)
                {
                    M5.Display.println("Failed to allocate memory for WAV file");
                    https.end();
                    return;
                }

                // Read in the data
                WiFiClient *stream = https.getStreamPtr();
                int totalRead = 0;
                while (https.connected() && totalRead < len)
                {
                    size_t availableBytes = stream->available();
                    if (availableBytes > 0)
                    {
                        int bytesRead = stream->read(buffer + totalRead, availableBytes);
                        if (bytesRead < 0)
                        {
                            break; // Error reading
                        }
                        totalRead += bytesRead;
                    }
                    delay(1); // Yield to let other tasks run
                }

                // Play the WAV file from the buffer
                M5.Speaker.playWav(buffer, totalRead);

                // Free the allocated buffer
                free(buffer);
            }
        }
        else
        {
            M5.Display.printf("HTTPS GET failed, code: %d\n", httpCode);
        }
        https.end();
    }
    else
    {
        M5.Display.println("HTTPS connection failed.");
    }
}