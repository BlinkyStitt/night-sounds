void setupSD() {
  // Wait for a SD card
  while (1) {
    if (SD.begin(SDCARD_CS_PIN)) {
      DEBUG_PRINTLN(F("SD card detected!"));
      break;
    }

    DEBUG_PRINTLN(F("Unable to access the SD card"));
    delay(1000);
  }
}
