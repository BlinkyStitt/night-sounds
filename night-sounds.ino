// TODO: this plays AAC and WAV files consistently, but MP3 has issues

#define DEBUG
#include <bs_debug.h>

#include <Audio.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <FastLED.h>
#include <IniFile.h>

#include <play_sd_aac.h>
#include <play_sd_mp3.h>

#define LED_DATA_PIN 0  // green // TODO: what pin. i want to share with the SD card
#define LED_CLOCK_PIN 1 // blue // TODO: what pin. i want to share with the SD card
#define SPI_MOSI_PIN 7  // Audio shield has MOSI on pin 7
#define SDCARD_CS_PIN 10
#define SPI_SCK_PIN 14  // Audio shield has SCK on pin 14
#define VOLUME_KNOB A2 // TODO: not attached!

#define LED_CHIPSET APA102
#define LED_MODE BGR

#define MAX_TRACKS 255

// Audio GUI - https://www.pjrc.com/teensy/gui/: begin automatically generated code
// TODO: wth. only the wav player seems to be working
AudioPlaySdWav           player_wav;     //xy=152,255
AudioPlaySdMp3           player_mp3;     //xy=154,197
AudioPlaySdAac           player_aac;     //xy=154,307
AudioMixer4              mixer_left;     //xy=415,210
AudioMixer4              mixer_right;    //xy=420,282
AudioMixer4              mixer_mono;     //xy=654.1538391113281,249.84613037109375
AudioOutputI2S           output_mono;    //xy=869,250
AudioConnection          patchCord1(player_wav, 0, mixer_left, 1);
AudioConnection          patchCord2(player_wav, 1, mixer_right, 1);
AudioConnection          patchCord3(player_mp3, 0, mixer_left, 0);
AudioConnection          patchCord4(player_mp3, 1, mixer_right, 0);
AudioConnection          patchCord5(player_aac, 0, mixer_left, 2);
AudioConnection          patchCord6(player_aac, 1, mixer_right, 2);
AudioConnection          patchCord7(mixer_left, 0, mixer_mono, 0);
AudioConnection          patchCord8(mixer_right, 0, mixer_mono, 1);
AudioConnection          patchCord9(mixer_mono, 0, output_mono, 0);
AudioConnection          patchCord10(mixer_mono, 0, output_mono, 1);
AudioControlSGTL5000     audioShield;    //xy=147,123
// Audio GUI - https://www.pjrc.com/teensy/gui/: end automatically generated code

// lights for compass ring
const int num_LEDs = 16;
CRGB leds[num_LEDs];

int frames_per_second = 30;  // TODO: read from SD
int default_brightness = 60;  // TODO: read from SD

// rotating "base color" used by some patterns
uint8_t g_hue = 0;

enum TrackCodec: byte {
  TRACK_AAC, TRACK_MP3, TRACK_WAV
};

typedef struct {
  TrackCodec codec;
  char filename[12];  // 8.3
} Track;

// TODO: change this to support multiple playlists
Track tracks[MAX_TRACKS];
int num_tracks = 0;
int current_track = 0;
int play_count = 0;

void setupSPI() {
  SPI.setMOSI(SPI_MOSI_PIN);
  SPI.setSCK(SPI_SCK_PIN);

  digitalWrite(SDCARD_CS_PIN, HIGH);

  delay(100); // give everything time to wake up

  SPI.begin();
}

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);

    delay(1000);
    while (!Serial) {
      ; // wait for serial port to connect so we can watch all prints during setup
    }
  #endif

  DEBUG_PRINTLN(F("Setting up..."));

  setupSPI();

  setupAudio();

  setupSD();

  setupLights();

  loadTracks();

  DEBUG_PRINTLN(F("Starting..."));
}

void loop() {
  playTrack();  // TODO: right now this calls updateLights, but I think that should be changed. instead it should check if playing and if not, play the next song

  updateLights();
}
