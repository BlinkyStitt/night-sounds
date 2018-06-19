// TODO: this plays AAC and WAV files consistently, but MP3 has issues

#define DEBUG
#include <bs_debug.h>

#include <Audio.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#include <play_sd_aac.h>
#include <play_sd_mp3.h>

#define SDCARD_CS_PIN 10
#define SPI_MOSI_PIN 7  // Audio shield has MOSI on pin 7
#define SPI_SCK_PIN 14  // Audio shield has SCK on pin 14
#define VOLUME_KNOB A2 // TODO: not attached!

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

void setupSD() {
  // Wait for a SD card
  while (1) {
    if (SD.begin(SDCARD_CS_PIN)) {
      DEBUG_PRINTLN("SD card detected!");
      break;
    }

    DEBUG_PRINTLN("Unable to access the SD card");
    delay(1000);
  }
}

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);

    delay(1000);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB
    }
  #endif

  DEBUG_PRINTLN("Setting up...");

  setupSPI();

  setupAudio();

  setupSD();

  loadTracks();

  DEBUG_PRINTLN("Starting...");
}

void loop() {
  playTrack();

  DEBUG_PRINTLN(F("Looping..."));
}
