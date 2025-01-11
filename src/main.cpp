// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <stdio.h>
#include <esp_heap_caps.h>
#include <esp32/spiram.h>

// header files
#include "initMic.h"
#include "drawWaveForm.h"
#include "initWifi.h"
#include "httpRequest.h"
#include "credentials.h"
#include "playAudio.h"
#include "createWavHeader.h"
#include "postWavFile.h"
#include "playAudioFromURL.h"

// Audio Recording Settings
static const uint32_t SAMPLE_RATE = 16000;                       // 16kHz
static const uint8_t BITS_PER_SAMPLE = 16;                       // 16-bit
static const uint8_t CHANNELS = 1;                               // Mono
static const uint32_t MAX_RECORD_SECS = 10;                      // Max record time
static const size_t MAX_SAMPLES = SAMPLE_RATE * MAX_RECORD_SECS; // 160k
static const size_t CHUNK_SIZE = 256;                            // Number of samples to fetch each time

// We'll store raw samples in a global buffer (16-bit each).
static int16_t *recordBuffer;
static size_t currentSampleIndex = 0;

void setupHomeScreen()
{
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(0, 0);
  M5.Display.println("Press and hold BtnA");
  M5.Display.println("to view waveform.");
}

// Add this helper function at the top of your file
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

  // M5 Unified init
  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);
  initWifi(ssid, password);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Check PSRAM
  if (!psramFound())
  {
    Serial.println("PSRAM not found or not enabled!");
  }

  // Allocate large buffer in PSRAM
  recordBuffer = (int16_t *)heap_caps_malloc(sizeof(int16_t) * 160000, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!recordBuffer)
  {
    Serial.println("Failed to allocate recordBuffer in PSRAM!");
  }
  else
  {
    Serial.println("Successfully allocated recordBuffer in PSRAM.");
  }

  // Print memory stats
  Serial.printf("Free PSRAM after allocation: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  // Optional: get current mic config and tweak it
  m5::mic_config_t micCfg = M5.Mic.config();
  micCfg.sample_rate = 16000; // Adjust sample rate if needed
  // micCfg.magnification = 32;      // Increase signal amplitude if desired
  // micCfg.noise_filter_level = 2;  // Add noise filtering if needed
  micCfg.magnification = 64;
  // Update config & begin microphone
  M5.Mic.config(micCfg);
  M5.Mic.begin();
  // M5.Speaker.begin();        // Initialize speaker
  // M5.Speaker.setVolume(255); // Set initial volume

  // Display some initial info
  setupHomeScreen();
}

void loop()
{
  // Update M5 (button states, etc.)
  M5.update();

  // Only draw the waveform if the A button is held down
  if (M5.BtnA.isHolding())
  {
    // if (!M5.Mic.isRecording())
    // {
    //   // Start or restart the recording session
    //   M5.Mic.begin();
    //   currentSampleIndex = 0;
    //   M5.Display.fillScreen(BLACK);
    //   M5.Display.setCursor(0, 0);
    //   M5.Display.println("Not Recording...");
    // }

    // Only grab another chunk if we have space
    if ((currentSampleIndex + CHUNK_SIZE) <= MAX_SAMPLES)
    {
      // Record CHUNK_SIZE samples (blocking call).
      bool ok = M5.Mic.record(&recordBuffer[currentSampleIndex], CHUNK_SIZE);
      if (!ok)
      {
        M5.Display.println("Record fail!");
        return;
      }

      // Draw waveform of just-grabbed chunk
      drawWaveform(&recordBuffer[currentSampleIndex], CHUNK_SIZE);

      currentSampleIndex += CHUNK_SIZE;
      M5.Display.setCursor(0, 80);
      M5.Display.clearDisplay();
      M5.Display.printf("Current sample index: %d\n", currentSampleIndex);
    }
    else
    {
      // We reached max length (10s). Indicate & do nothing more.
      M5.Display.setCursor(0, 80);
      M5.Display.println("Max length reached!");
    }
  }
  else if (M5.BtnA.wasReleased())
  {
    // M5.Mic.end();
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
    //    We'll allocate a temporary buffer in PSRAM or normal RAM for the WAV.
    //    Enough for a 44-byte header + raw PCM data.
    size_t rawBytes = totalSamples * sizeof(int16_t);
    size_t wavTotalLength = 44 + rawBytes;

    // Allocate a buffer large enough for WAV data
    uint8_t *wavBuffer = (uint8_t *)heap_caps_malloc(wavTotalLength, MALLOC_CAP_DEFAULT);
    if (!wavBuffer)
    {
      M5.Display.println("Error: Out of memory for WAV buffer!");
      return;
    }

    // Write the WAV header (returns how many bytes written, typically 44)
    size_t headerSize = createWavHeader(
        wavBuffer,
        rawBytes,
        SAMPLE_RATE,
        BITS_PER_SAMPLE,
        CHANNELS);

    // Copy raw PCM data right after the header
    memcpy(wavBuffer + headerSize, recordBuffer, rawBytes);

    // 2) Send the WAV via HTTP POST
    char *aiResponse = postWavData(wavBuffer, wavTotalLength);

    M5.Display.printf("AI Response: %s\n", aiResponse);

    playAudioFromUrl(aiResponse);

    // Free the WAV buffers
    heap_caps_free(wavBuffer);

    // Reset for next time
    currentSampleIndex = 0;
    M5.Display.println("Done!");
    // Clear the waveform area when the button is released
    // setupHomeScreen();
  }
  if (M5.BtnB.isPressed())
  {
    resetAudioSystem();
    playAudioFromUrl("Hello Alyssa");
  }
  if (M5.BtnC.isPressed())
  {
    resetAudioSystem();
    playAudio("https://speech-to-text-recordings-esp32.s3.us-east-2.amazonaws.com/audio/1736541486772-speech-elevenlabs.wav?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIATCKANPUGQOEUCMN4%2F20250110%2Fus-east-2%2Fs3%2Faws4_request&X-Amz-Date=20250110T203806Z&X-Amz-Expires=3600&X-Amz-Signature=6501716b70cc48b259fbbcd47da99e1c79cf7c9258a4b5663fb761bb44544386&X-Amz-SignedHeaders=host&x-id=GetObject");
  }
}
