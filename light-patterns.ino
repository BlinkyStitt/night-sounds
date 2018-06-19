// TODO: peaceful patterns for night lights/porta potty

void circle() {
  int ms_per_led = 3 * 1000 / frames_per_second;  // 3 frames
  int pos = (millis() / ms_per_led) % num_LEDs;  // TODO: use now_millis so we stay in sync with others?

  fadeToBlackBy(leds, num_LEDs, 64);

  leds[pos] += CHSV(g_hue, 255, 192);

  pos++;
  if (pos >= num_LEDs) {
    pos = 0;
  }
}
