#include <Arduino.h>
#include "M5Unified.h"

void initMic()
{
    M5.Display.fillRect(0, 40, 320, 80, BLACK); // Clear previous messages

    // Initialize microphone with configurations
    auto cfg = M5.Mic.config();
    M5.Display.setTextColor(YELLOW);
    M5.Display.setCursor(20, 40);
    M5.Display.println("Setting config...");

    cfg.sample_rate = 16000; // 16KHz sample rate
    cfg.dma_buf_count = 8;
    cfg.dma_buf_len = 256;
    cfg.pin_data_in = 34; // Mic data pin

    M5.Mic.config(cfg);

    M5.Display.setCursor(20, 60);
    M5.Display.println("Starting mic...");

    // Start the microphone
    bool began = M5.Mic.begin();
    M5.Display.setTextColor(began ? GREEN : RED);
    M5.Display.setCursor(20, 80);
    M5.Display.printf("Begin result: %s", began ? "SUCCESS" : "FAILED");

    delay(100); // Give it a moment to start
    M5.Display.setCursor(20, 100);
    M5.Display.printf("Record status: %d", M5.Mic.isRecording());
}