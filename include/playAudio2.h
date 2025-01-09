// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "credentials.h"

// void playAudio(const char *url)
void playAudio2()

{

    WiFiClientSecure *client = new WiFiClientSecure;
    M5.Display.println("making HTTP request");

    if (!client)
    {
        M5.Display.println("[HTTPS] Failed to create client");
        return;
    }
    // import root certificate

    client->setCACert(rootCACertificate);
    client->setHandshakeTimeout(30); // Give more time for handshake

    HTTPClient https;

    https.setTimeout(5000); // e.g., 5 seconds

    if (!https.begin(*client, ttsEndpoint))
    {
        M5.Display.println("Connection to TTS endpoint failed");
        return;
    }

    // Add JSON header if your TTS endpoint expects JSON body
    https.addHeader("Content-Type", "application/json");

    char text[] = "Hello!";

    // Build JSON body like: { "text": "Hello from M5" }
    String requestBody = String("{\"text\":\"") + text + "\"}";

    // POST the text to your TTS endpoint
    int httpCode = https.POST(requestBody);

    if (httpCode != HTTP_CODE_OK)
    {
        M5.Display.printf("HTTP error code: %d\n", httpCode);
        https.end();
        return;
    }

    M5.Display.println("HTTP POST request sent");

    // Grab the stream pointer to read chunks
    WiFiClient *stream = https.getStreamPtr();

    const size_t MAX_BUFFER = 400000; // ~400KB
    size_t totalRead = 0;
    uint8_t *audioBuffer = (uint8_t *)malloc(MAX_BUFFER);
    if (!audioBuffer)
    {
        M5.Display.println("Not enough memory for audio buffer!");
        https.end();
        return;
    }

    M5.Display.println("Reading audio data...");

    unsigned long lastDataMillis = millis();
    const unsigned long NO_DATA_TIMEOUT = 1000; // 1 seconds

    while (https.connected() && (totalRead < MAX_BUFFER))
    {
        size_t availableBytes = stream->available();
        if (availableBytes > 0)
        {
            int bytesRead = stream->read(audioBuffer + totalRead, availableBytes);
            if (bytesRead < 0)
            {
                M5.Display.println("Error while reading TTS audio data");
                break;
            }
            if (bytesRead > 0)
            {
                totalRead += bytesRead;
                lastDataMillis = millis(); // reset timer because we got data
            }
        }
        else
        {
            // No data right now, small delay
            delay(5);

            // Check how long it's been since we last got data
            if (millis() - lastDataMillis > NO_DATA_TIMEOUT)
            {
                M5.Display.println("No data for too long, stopping.");
                break;
            }
        }
    }

    // Read raw PCM chunks until the connection closes or we exceed MAX_BUFFER
    // while (https.connected() && (totalRead < MAX_BUFFER))
    // {
    //     // Check how many bytes are available to read
    //     size_t availableBytes = stream->available();
    //     if (availableBytes > 0)
    //     {
    //         // Read into our buffer
    //         int bytesRead = stream->read(audioBuffer + totalRead, availableBytes);
    //         if (bytesRead <= 0)
    //         {
    //             M5.Display.println("Error while reading TTS audio data");
    //             break;
    //         }
    //         totalRead += bytesRead;
    //     }
    //     else
    //     {
    //         // Nothing available right now, short delay
    //         // delay(50);
    //     }
    // }

    // totalRead now contains the raw PCM data length in bytes.

    M5.Display.printf("Downloaded %d bytes of PCM audio\n", totalRead);

    https.end();

    if (totalRead > 0)
    {
        // We know it's 16-bit, mono, and sample rate = 22050 (example).
        // So we can call playRaw() with int16_t pointer.
        // The `array_len` in M5.Speaker.playRaw() is the number of bytes in the buffer (according to docs).
        // We'll pass totalRead directly.
        bool success = M5.Speaker.playRaw(
            (int16_t *)audioBuffer,
            totalRead,
            22000, // or whatever sample rate your TTS uses
            false, // mono = false, stereo = true
            1,     // number of times to repeat
            -1,    // any free channel
            true   // stop current sound (if any) and start playback
        );

        if (!success)
        {
            M5.Display.println("Failed to start raw playback");
        }
    }
    else
    {
        M5.Display.println("No audio data received");
    }

    free(audioBuffer);
    return;
}
