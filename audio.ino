void setupAudio() {
  audioShield.enable();

  // TODO: tune this
  audioShield.volume(0.5);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  // TODO: tune this
  AudioMemory(20);

  // put the gain a bit lower, some MP3 files will clip otherwise.
  mixer_left.gain(0, 0.9);
  mixer_right.gain(0, 0.9);

  mixer_mono.gain(0, 0.5);
  mixer_mono.gain(1, 0.5);

  // TODO: set line output levels
}

bool is_hidden_file(String filename) {
  // TODO: i don't think we need the check for '.'
  return filename.startsWith('_') or filename.startsWith('.');
}

bool is_mp3(String filename) {
  return filename.endsWith(F(".MP3"));
}

bool is_aac(String filename) {
  return filename.endsWith(F(".AAC")) or filename.endsWith(F(".MP4")) or filename.endsWith(F(".M4A"));
}

bool is_wav(String filename) {
  return filename.endsWith(F(".WAV"));
}

// TODO: take args so this can load music, meditations, and sound effects
void loadTracks() {
  static File root;

  // Starting to index the SD card for MP3/AAC.
  // TODO: load /music, /meditations, and /noises
  root = SD.open("/");

  while (true) {
    File file = root.openNextFile();
    if (!file) {
      // If no more files, break out.
      break;
    }
    String file_string = file.name(); // put file in string

    DEBUG_PRINT(F("Found: "));
    DEBUG_PRINT(file_string);

    if (is_hidden_file(file_string)) {
      DEBUG_PRINTLN(F(" (Skipped)"));

      file.close();
      continue;
    }

    if (is_mp3(file_string)) {
      DEBUG_PRINTLN(F(" (MP3)"));
      tracks[num_tracks].codec = TRACK_MP3;
    } else if (is_aac(file_string)) {
      DEBUG_PRINTLN(F(" (AAC)"));
      tracks[num_tracks].codec = TRACK_AAC;
    } else if (is_wav(file_string)) {
      DEBUG_PRINTLN(F(" (WAV)"));
      tracks[num_tracks].codec = TRACK_WAV;
    } else {
      DEBUG_PRINTLN(F(" (Skipped)"));

      file.close();
      continue;
    }

    // TODO: wth. the .WAV file is getting another name stuck to the end of it
    strcpy(tracks[num_tracks].filename, file.name());

    file.close();

    DEBUG_PRINT(F("Saved to track list as: #"));
    DEBUG_PRINT(num_tracks);
    DEBUG_PRINT(F(" "));
    DEBUG_PRINTLN(tracks[num_tracks].filename);

    num_tracks++;
  }

  // make sure EEPROM/current_track have a valid value
  // TODO: instead of reading/writing to 0, write to an enum/DEFINE
  current_track = EEPROM.read(0);
  if (current_track > num_tracks) {
    EEPROM.write(0, 0);
    current_track = 0;
  }

  // TODO: instead of reading/writing to 1, write to an enum/DEFINE
  play_count = EEPROM.read(1);

  DEBUG_PRINT(F("current_track: "));
  DEBUG_PRINTLN(current_track);

  DEBUG_PRINT(F("play_count: "));
  DEBUG_PRINTLN(play_count);

  DEBUG_PRINT(F("num_tracks: "));
  DEBUG_PRINTLN(num_tracks);
}

void printAudioSummary() {
  EVERY_N_SECONDS(1) {
    // print a summary of the current & maximum usage
    DEBUG_PRINT("CPU: ");
    DEBUG_PRINT("mp3=");
    DEBUG_PRINT(player_mp3.processorUsage());
    DEBUG_PRINT(",");
    DEBUG_PRINT(player_mp3.processorUsageMax());
    DEBUG_PRINT("  ");
    DEBUG_PRINT("aac=");
    DEBUG_PRINT(player_aac.processorUsage());  // TODO: this always returns 0
    DEBUG_PRINT(",");
    DEBUG_PRINT(player_aac.processorUsageMax()); // TODO: this always returns 0
    DEBUG_PRINT("  ");
    DEBUG_PRINT("wav=");
    DEBUG_PRINT(player_wav.processorUsage());
    DEBUG_PRINT(",");
    DEBUG_PRINT(player_wav.processorUsageMax());
    DEBUG_PRINT("  ");
    DEBUG_PRINT("all=");
    DEBUG_PRINT(AudioProcessorUsage());
    DEBUG_PRINT(",");
    DEBUG_PRINT(AudioProcessorUsageMax());
    DEBUG_PRINT("    ");
    DEBUG_PRINT("Memory: ");
    DEBUG_PRINT(AudioMemoryUsage());
    DEBUG_PRINT(",");
    DEBUG_PRINT(AudioMemoryUsageMax());
    DEBUG_PRINTLN();
  }
}

int music_play_count = 1;

// pass play_count and current_track by reference so we can support multiple playlists
void playTrack() {
  const int track_id = current_track;

  // prepare the next track if we have played this one enough times
  // TODO: increment after we start playing so we can handle the file failing to play
  play_count++;
  DEBUG_PRINT(F("Play count: "));
  DEBUG_PRINTLN(play_count);
  if (play_count >= music_play_count) {
    play_count = 0;

    current_track++;
    if (current_track >= num_tracks) {
      current_track = 0;
    }
    DEBUG_PRINT(F("Saving next track to EEPROM: "));
    DEBUG_PRINTLN(current_track);
    EEPROM.write(0, current_track);
  }
  EEPROM.write(1, play_count);

  DEBUG_PRINT(F("Playing track: #"));
  DEBUG_PRINT(track_id);
  DEBUG_PRINT(F(" "));
  DEBUG_PRINT(tracks[track_id].filename);

  // Start playing the file. This sketch continues to run while the file plays.
  // TODO: dry this up
  // TODO: play WAV/OGG/FLAC files, too?
  // TODO: don't wait here so that we can handle lights/sensors well. or atleast call updatelights/sensors in a loop
  switch (tracks[track_id].codec) {
  case (TRACK_AAC):
    DEBUG_PRINTLN(F(" (as AAC)"));

    // TODO: none of my example AAC tracks are playing!
    player_aac.play(tracks[track_id].filename);

    // A brief delay for the library read file info
    FastLED.delay(5);

    // Simple wait for the file to finish playing.
    while (player_aac.isPlaying()) {
      printAudioSummary();
      updateLights();
    }
    break;
  case (TRACK_MP3):
    DEBUG_PRINTLN(F(" (as MP3)"));

    // TODO: none of my example MP3 tracks are playing!
    player_mp3.play(tracks[track_id].filename);

    // A brief delay for the library read file info
    FastLED.delay(5);

    /*
    // TODO: retry mp3 as aac?
    if (! player_mp3.isPlaying()) {
      play_count--;
      current_track--;  // TODO: this isn't saved to EEPROM, but that's okay for now

      // the player isn't playing only 5ms after starting
      // sometimes .MP3 files are actually encoded with AAC
      tracks[track_id].codec = TRACK_AAC;

      // TODO: rename the file to .AAC for next boot?
      playTrack();
      // TODO: this is still failing sometimes
      return;
    }
    */

    // Simple wait for the file to finish playing.
    // TODO: this is exiting immediatly. try a different file and try wav
    while (player_mp3.isPlaying()) {
      printAudioSummary();
      updateLights();
    }
    break;
  case (TRACK_WAV):
    DEBUG_PRINTLN(F(" (as WAV)"));

    player_wav.play(tracks[track_id].filename);

    // A brief delay for the library read file info
    FastLED.delay(5);

    // Simple wait for the file to finish playing.
    while (player_wav.isPlaying()) {
      printAudioSummary();
      updateLights();
    }
    break;
  }
}
