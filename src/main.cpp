// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <stdio.h>
#include <esp_heap_caps.h>
#include <esp32/spiram.h>

// header files
#include "drawWaveForm.h"
#include "initWifi.h"
#include "httpRequest.h"
#include "credentials.h"
#include "playAudio.h"
#include "createWavHeader.h"
#include "postWavFile.h"
#include "playAudioFromText.h"

// Audio Recording Settings
static const uint32_t SAMPLE_RATE = 8000;  // 8KHz
static const uint8_t BITS_PER_SAMPLE = 16; // 16-bit
static const uint8_t CHANNELS = 1;         // Mono
static const uint32_t MAX_RECORD_SECS = 5;
static const size_t MAX_SAMPLES = SAMPLE_RATE * MAX_RECORD_SECS; // 44,100
static const size_t CHUNK_SIZE = 256;

// Raw samples in a global buffer
static int16_t *recordBuffer = nullptr;
static size_t currentSampleIndex = 0;

void setupHomeScreen()
{
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(0, 0);
  M5.Display.println("Press and hold BtnA");
  M5.Display.println("to view waveform.");
}

void resetAudioSystem()
{
  M5.Speaker.stop();
  M5.Speaker.end();
  delay(100);
  M5.Speaker.begin();
  M5.Speaker.setVolume(128);
}

void setup()
{
  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);
  initWifi(ssid, password);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  recordBuffer = (int16_t *)heap_caps_malloc(sizeof(int16_t) * MAX_SAMPLES,
                                             MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!recordBuffer)
  {
    M5.Display.println("Failed to allocate recordBuffer in PSRAM!");
  }
  else
  {
    M5.Display.println("Successfully allocated recordBuffer in PSRAM.");
  }

  // Print memory stats
  M5.Display.printf("Free PSRAM after allocation: %u bytes\n",
                    heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  m5::mic_config_t micCfg = M5.Mic.config();
  micCfg.sample_rate = SAMPLE_RATE;
  micCfg.magnification = 64;
  M5.Mic.config(micCfg);
  M5.Mic.begin();

  setupHomeScreen();
}

void loop()
{
  M5.update();

  // Only record/draw waveform if the A button is held down
  if (M5.BtnA.isHolding())
  {
    if ((currentSampleIndex + CHUNK_SIZE) <= MAX_SAMPLES)
    {
      bool ok = M5.Mic.record(&recordBuffer[currentSampleIndex], CHUNK_SIZE);
      if (!ok)
      {
        M5.Display.println("Record fail!");
        return;
      }

      drawWaveform(&recordBuffer[currentSampleIndex], CHUNK_SIZE);

      currentSampleIndex += CHUNK_SIZE;
    }
    else
    {
      M5.Display.setCursor(0, 80);
      M5.Display.println("Max length (6s) reached!");
    }
  }
  else if (M5.BtnA.wasReleased()) // stop recording and send the data
  {
    M5.Mic.end();
    resetAudioSystem();

    size_t totalSamples = currentSampleIndex;

    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Button released");
    M5.Display.println("Creating WAV & sending...");

    if (totalSamples == 0)
    {
      M5.Display.println("No samples recorded!");
      return;
    }

    // 1) Convert the recorded PCM data into a WAV file
    size_t rawBytes = totalSamples * sizeof(int16_t);
    size_t wavTotalLength = 44 + rawBytes;

    // Allocate a buffer large enough for WAV data
    uint8_t *wavBuffer = (uint8_t *)heap_caps_malloc(wavTotalLength, MALLOC_CAP_DEFAULT);

    if (!wavBuffer)
    {
      M5.Display.println("Error: Out of memory for WAV buffer!");
      return;
    }

    // Write the WAV header
    size_t headerSize = createWavHeader(
        wavBuffer,
        rawBytes,
        SAMPLE_RATE,
        BITS_PER_SAMPLE,
        CHANNELS);

    // Copy raw PCM data right after the header
    // memcpy(wavBuffer + headerSize, recordBuffer, rawBytes);

    // 2) Send the WAV to server and get a text response from AI
    char *aiResponse = postWavData(wavBuffer, wavTotalLength);

    M5.Display.printf("AI Response: %s\n", aiResponse);

    playAudioFromText(aiResponse);

    // Free the WAV buffer
    heap_caps_free(wavBuffer);

    // Reset for next time
    currentSampleIndex = 0;
    setupHomeScreen();
  }

  if (M5.BtnB.isPressed())
  {
    playAudioFromText("Hello My name is Jarvis");
  }
}
