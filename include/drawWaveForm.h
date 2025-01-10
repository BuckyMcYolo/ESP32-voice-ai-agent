#include <Arduino.h>
#include "M5Unified.h"

void drawWaveform(const int16_t *samples, size_t numSamples)
{
    // Clear upper portion of the screen (for a nicer "scrolling" effect).
    // Adjust to your preference. Right now, just blank out a zone.
    M5.Display.fillRect(0, 50, 320, 150, BLACK);

    // Middle of the waveform area for vertical offset
    int16_t midY = 125; // Some offset down the screen

    // Draw lines from sample to sample
    for (int i = 0; i < ((int)numSamples - 1); i++)
    {
        int x1 = map(i, 0, numSamples - 1, 0, 319);
        int y1 = map(samples[i], -32768, 32767, midY + 70, midY - 70);

        int x2 = map(i + 1, 0, numSamples - 1, 0, 319);
        int y2 = map(samples[i + 1], -32768, 32767, midY + 70, midY - 70);

        M5.Display.drawLine(x1, y1, x2, y2, GREEN);
    }
}