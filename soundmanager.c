/*
 * $Id: soundmanager.c,v 1.3 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * BGM/SE manager(using SDL_mixer).
 *
 * @version $Revision: 1.3 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "SDL_mixer.h"
#include "soundmanager.h"



static int useAudio = 0;

#define MUSIC_NUM 3
#define SHARE_LOC "uda://rrootage/"

static char *musicFileName[MUSIC_NUM] = {
  "stg_a.ogg", "stg_b.ogg", "stg_c.ogg",
};
static Mix_Music *music[MUSIC_NUM];

#define CHUNK_NUM 16

static char *chunkFileName[CHUNK_NUM] = {
  "laser_start.wav", "laser.wav", "damage.wav", "bomb.wav",
  "destroied.wav", "explosion1.wav", "explosion2.wav", "miss.wav", "extend.wav",
  "grz.wav", "grzinv.wav",
  "shot.wav", "change.wav",
  "reflec1.wav", "reflec2.wav", "ref_ready.wav",
};
static Mix_Chunk *chunk[CHUNK_NUM];
static int chunkChannel[CHUNK_NUM] = {
  0, 1, 2, 3,
  4, 5, 6, 7, 4,
  6, 7,
  6, 7,
  7, 7, 7,
};
void closeSound() {
  int i;
  if ( !useAudio ) return;
  if ( Mix_PlayingMusic() ) {
    Mix_HaltMusic();
  }
  for ( i=0 ; i<MUSIC_NUM ; i++ ) {
    if ( music[i] ) {
      Mix_FreeMusic(music[i]);
    }
  }
  for ( i=0 ; i<CHUNK_NUM ; i++ ) {
    if ( chunk[i] ) {
      Mix_FreeChunk(chunk[i]);
    }
  }
  Mix_CloseAudio();
}


// Initialize the sound.

static void loadSounds() {
	//senquack
	//printf("entering loadSounds()\n");
	//fflush(stdout);

  int i;
  char name[128];

  for ( i=0 ; i<MUSIC_NUM ; i++ ) {
    strcpy(name, SHARE_LOC);
    strcat(name, "sounds/");
    strcat(name, musicFileName[i]);

		//senquack
		//printf("trying to load %s in loadSounds()\n", name);
		//fflush(stdout);

    if ( NULL == (music[i] = Mix_LoadMUS(name)) ) {
      printf("Couldn't load: %s\n", name);
		printf("Error: %s\n",Mix_GetError());
		//fflush(stdout);
      useAudio = 0;
      return;
    }
  }

	//senquack
	//printf("loaded music in loadSounds()\n");
	//fflush(stdout);

  for ( i=0 ; i<CHUNK_NUM ; i++ ) {
    strcpy(name, SHARE_LOC);
    strcat(name, "/sounds/");
    strcat(name, chunkFileName[i]);
    if ( NULL == (chunk[i] = Mix_LoadWAV(name)) ) {
      fprintf(stderr, "Couldn't load: %s\n", name);
      useAudio = 0;
      return;
    }
  }

	//senquack
	//printf("leaving loadSounds()\n");
	//fflush(stdout);
}

void initSound() {
  int audio_rate;
  Uint16 audio_format;
  int audio_channels;
  int audio_buffers;

  if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 ) {
    fprintf(stderr, "Unable to initialize SDL_AUDIO: %s\n", SDL_GetError());
    return;
  }

//  audio_rate = 44100;
//  audio_format = AUDIO_S16;
//  audio_channels = 1;
//  audio_buffers = 4096;
  audio_rate = 48000;
  audio_format = AUDIO_S16MSB;
  audio_channels = 2;
  audio_buffers = 2048;

  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return;
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  }


  useAudio = 1;
  loadSounds();
}

// Play/Stop the music/chunk.

void playMusic(int idx) {
  if ( !useAudio ) return;
    char name[32];
    strcpy(name, musicFileName[idx]);
  //fprintf(stderr, "trying to play music file: %s\n", name);
  Mix_PlayMusic(music[idx], -1);
}

void fadeMusic() {
  if ( !useAudio ) return;
  Mix_FadeOutMusic(1280);
}

void stopMusic() {
  if ( !useAudio ) return;
  if ( Mix_PlayingMusic() ) {
    Mix_HaltMusic();
  }
}

void playChunk(int idx) {
  if ( !useAudio ) return;
  Mix_PlayChannel(chunkChannel[idx], chunk[idx], 0);
}

void haltChunk(int idx) {
  if ( !useAudio ) return;
  Mix_HaltChannel(chunkChannel[idx]);
}
