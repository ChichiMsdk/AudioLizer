#ifndef EDITOR_H
#define EDITOR_H

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN 
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <SDL3/SDL.h>
#include "SDL3/SDL_audio.h"

// #include <SDL3_ttf/SDL_ttf.h>
// #include <SDL3_mixer/SDL_mixer.h>

#include "vector.h"

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0

/*
 * typedef struct sfx
 * {
 * 	Mix_Chunk	*pressed;
 * 	Mix_Chunk 	*released;
 * 	int			index;
 * 	bool		is_pressed;
 * }sfx;
 */

typedef struct AudioData
{
    Uint8 *buffer;
    Uint32 length;
    Uint32 position;
} AudioData;

typedef struct wav_header
{
  char riff[4];           /* "RIFF"                                  */
  int32_t flength;        /* file length in bytes                    */
  char wave[4];           /* "WAVE"                                  */
  char fmt[4];            /* "fmt "                                  */
  int32_t chunk_size;     /* size of FMT chunk in bytes (usually 16) */
  int16_t format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
  int16_t num_chans;      /* 1=mono, 2=stereo                        */
  int32_t srate;          /* Sampling rate in samples per second     */
  int32_t bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
  int16_t bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
  int16_t bits_per_samp;  /* Number of bits per sample               */
  char data[4];           /* "data"                                  */
  int32_t dlength;        /* data length in bytes (filelength - 44)  */
}t_wav;

// Check padding
typedef struct YUinstance
{
	SDL_Window			*window;
	SDL_Renderer		*renderer;
	SDL_Rect			rect;
	SDL_Event			e;
	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;
	FILE				*audio_file;
}YUinstance;

// globals
extern YUinstance	g_inst;
extern int			WINDOW_WIDTH;
extern int			WINDOW_HEIGHT;
extern int			g_retrieving;
extern int			g_running;
extern t_wav		g_wav_header;

// editor.c
void				Events(SDL_Event e);
void				save_file(FILE *file);
void				cleanup(void);
void				retrieve_stream_data(void);

// audio_setup.c 
int 				get_audio_capture_id(char *device_name);
int 				get_audio_output_id(char *device_name);
void 				wav_header_init(SDL_AudioSpec audio_spec);
SDL_AudioSpec		set_capture_device(char *device_name);
SDL_AudioSpec 		set_output_device(char *device_name);
SDL_AudioStream		*stream_capture_init(SDL_AudioSpec a_spec, 
		SDL_AudioDeviceID logical_dev_id);

// error.c
void				print_audio_spec_info(SDL_AudioSpec micSpec, int micSample);
void				logExit(char *msg);
void				print_stream_format();


							// window renderer surface font
void				logger(void *w, void *r, void *s, void *f, const char *msg);
#endif
