void setupLights() {
  DEBUG_PRINT(F("Setting up lights... "));

  pinMode(LED_DATA_PIN, OUTPUT);

  // TODO: seed fastled random?

  // https://learn.adafruit.com/adafruit-feather-m0-basic-proto/power-management
  // While you can get 500mA from it, you can't do it continuously from 5V as it will overheat the regulator.
  FastLED.setMaxPowerInVoltsAndMilliamps(3.3, 500);

  FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_CLOCK_PIN, LED_MODE>(leds, num_LEDs).setCorrection(TypicalSMD5050);

  FastLED.setBrightness(default_brightness); // TODO: read this from the SD card
  FastLED.clear();
  FastLED.show();

  DEBUG_PRINTLN(F("done."));
}

void updateLights() {
  // update the led array every frame
  // TODO: EVERY_N_* isn't working while the music is playing :(
  EVERY_N_MILLISECONDS(1000 / frames_per_second) {

    // TODO: read SD to know what pattern to use
    // TODO: make lights optional. make them able to turn off after X minutes
    //circle();
    colorTwinkles();

    #ifdef DEBUG
      // debugging lights
      int now = millis() % 10000;
      if (now < 1000) {
        DEBUG_PRINT(F(" "));

        if (now < 100) {
          DEBUG_PRINT(F(" "));

          if (now < 10) {
            DEBUG_PRINT(F(" "));
          }
        }
      }
      DEBUG_PRINT(now);

      DEBUG_PRINT(F(": "));
      for (int i = 0; i < num_LEDs; i++) {
        if (leds[i]) {
          // TODO: better logging?
          DEBUG_PRINT(F("X"));
        } else {
          DEBUG_PRINT(F("O"));
        }
      }
      DEBUG_PRINTLN();
    #endif

    // display the colors
    FastLED.show();
  }

  EVERY_N_MILLISECONDS(3 * 1000 / frames_per_second) {
    // slowly cycle the color
    g_hue++;
  }
}
