// source files
#include <Arduino.h>
#include "M5Unified.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <stdio.h>

// header files
#include "initMic.h"
#include "drawWaveForm.h"
#include "initWifi.h"
#include "httpRequest.h"
#include "credentials.h"
#include "playAudio.h"

void setupHomeScreen()
{
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(0, 0);
  M5.Display.println("Press and hold BtnA");
  M5.Display.println("to view waveform.");
}

void setup()
{

  // M5 Unified init
  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);
  initWifi(ssid, password);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Optional: get current mic config and tweak it
  m5::mic_config_t micCfg = M5.Mic.config();
  micCfg.sample_rate = 16000; // Adjust sample rate if needed
  // micCfg.magnification = 32;      // Increase signal amplitude if desired
  // micCfg.noise_filter_level = 2;  // Add noise filtering if needed
  micCfg.magnification = 64;
  // Update config & begin microphone
  M5.Mic.config(micCfg);
  M5.Mic.begin();
  M5.Speaker.begin();        // Initialize speaker
  M5.Speaker.setVolume(255); // Set initial volume

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
    drawMicrophoneWaveform();
  }
  else if (M5.BtnA.wasReleased())
  {
    // Clear the waveform area when the button is released
    setupHomeScreen();
  }
  if (M5.BtnB.isPressed())
  {
    playAudio();
  }
}
