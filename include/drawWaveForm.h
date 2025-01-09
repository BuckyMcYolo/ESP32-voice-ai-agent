#include <Arduino.h>
#include "M5Unified.h"

static const size_t NUM_SAMPLES = 256;
static int16_t samples[NUM_SAMPLES];

// Encapsulated function to record audio and draw its waveform
void drawMicrophoneWaveform()
{
    // Attempt to record one frame of data (blocking call)
    bool success = M5.Mic.record(samples, NUM_SAMPLES);
    if (!success)
    {
        // If recording failed, just return
        return;
    }

    // Clear a portion of the screen (or the whole screen) for the waveform
    M5.Display.fillScreen(BLACK);

    // Middle of the waveform area for vertical offset
    int16_t midY = 50 + 70;

    // Draw waveform
    for (int i = 0; i < NUM_SAMPLES - 1; i++)
    {
        int x1 = map(i, 0, NUM_SAMPLES - 1, 0, 319);
        int y1 = map(samples[i], -32768, 32767, midY + 70, midY - 70);
        int x2 = map(i + 1, 0, NUM_SAMPLES - 1, 0, 319);
        int y2 = map(samples[i + 1], -32768, 32767, midY + 70, midY - 70);

        // (Optional) Clamp y1, y2 so they don't exceed the screen bounds
        // if (y1 < 0) y1 = 0; else if (y1 > 239) y1 = 239;
        // if (y2 < 0) y2 = 0; else if (y2 > 239) y2 = 239;

        M5.Display.drawLine(x1, y1, x2, y2, GREEN);
    }
}
