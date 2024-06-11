#ifndef AUDIO_H
#define AUDIO_H

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#define SDL_MAIN_HANDLED

#include <SDL3/SDL_audio.h>

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

// #include <SDL3_ttf/SDL_ttf.h>
// #include <SDL3_mixer/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "vector.h"
#include "gui.h"
#include "camera.h"

#define MAX_BUFFER_SIZE 2000000000
#define FIRST_ALLOC 1000 * 100

/*
 * typedef struct sfx
 * {
 * 	Mix_Chunk	*pressed;
 * 	Mix_Chunk 	*released;
 * 	int			index;
 * 	bool		is_pressed;
 * }sfx;
 */

typedef enum 
{
	OUTPUT = 0,
	CAPTURE = 1
}DeviceType;

typedef struct wav_header
{
  char					riff[4];           /* "RIFF"                                  */
  int32_t				flength;           /* file length in bytes                    */
  char					wave[4];           /* "WAVE"                                  */
  char					fmt[4];            /* "fmt "                                  */
  int32_t				chunk_size;        /* size of FMT chunk in bytes (usually 16) */
  int16_t				format_tag;        /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
  int16_t				num_chans;         /* 1=mono, 2=stereo                        */
  int32_t				srate;             /* Sampling rate in samples per second     */
  int32_t				bytes_per_sec;     /* bytes per second = srate*bytes_per_samp */
  int16_t				bytes_per_samp;    /* 2=16-bit mono, 4=16-bit stereo          */
  int16_t				bits_per_samp;     /* Number of bits per sample               */
  char					data[4];           /* "data"                                  */
  int32_t				dlength;           /* data length in bytes (filelength - 44)  */
}t_wav;

/* make two distinct types ? import and export */
typedef struct AudioData
{
	SDL_AudioSpec		spec;
	const char			*path;
    Uint8				*buffer;
    Uint32				length;
    Uint32				position;
	SDL_AudioStream 	*stream;
	int					sample_size;
	size_t				current_buff_size;
	t_wav				header;
	bool				paused;

	int					index;
	Uint8				is_pressed;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;
} AudioData;

typedef struct LogicalDevice
{
	SDL_AudioDeviceID	logical_id;
	SDL_AudioDeviceID	physical_id;
	DeviceType			type;
	int					sample;
	SDL_AudioSpec		spec;
	SDL_AudioStream 	*stream;
	const char			*name; /* not guaranteed to get the device */
    /*
	 * void				*buffer;
	 * size_t				current_buff_size;
     */
}LogicalDevice;

/* Check padding */
typedef struct YUinstance
{
	SDL_Window			*window;
	SDL_Renderer		*renderer;
	SDL_Rect			rect;
	SDL_Event			e;
	SDL_Texture			*texture;

	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;

	FILE				*audio_file;
	char				*capture_name;
	char				*output_name;
	int					sample_size;
	size_t				current_buff_size;

	Camera2D			*cam;
	SDL_Cursor			*cursor;
	Button				button;
}YUinstance;

// globals
extern YUinstance		g_inst;
extern int				WINDOW_WIDTH;
extern int				WINDOW_HEIGHT;
extern int				g_retrieving;
extern int				g_vizualizing;
extern int				g_running;
extern t_wav			g_wav_header;
extern int				g_sending;
extern int				g_BUFF_SIZE;

// editor.c
Mouse_state				get_mouse_state(void);
void					Events(SDL_Event e, AudioData *a_data);
void					cleanup(void);
void					make_realtime_plot(const void *buffer, size_t length);

// audio_setup.c 
void					init_wav_header(t_wav *header, SDL_AudioSpec audio_spec);
int						get_audio_device_id(const char *device_name, DeviceType type);
SDL_AudioSpec			set_audio_device(LogicalDevice *device);
void					init_audio_device(LogicalDevice *device, const char *name,
												DeviceType type, SDL_AudioSpec spec);

AudioData				link_data_capture(LogicalDevice device, SDL_AudioStream *stream,
												SDL_AudioSpec spec);

SDL_AudioStream*		init_audio_stream(LogicalDevice *device, 
												SDL_AudioSpec spec, DeviceType type);
//file_process.c
void					adjust_volume_for_file(float factor, uint8_t *buffer, int32_t length);
void					save_file(char *file_name, AudioData *a_data);
void					retrieve_stream_data(AudioData *audio_data, 
												SDL_AudioStream *stream, int visu);
AudioData				load_wav(const char *fpath);
//button.c
void					draw_button(void);
Mouse_state				get_mouse_state(void);
void					vizualize_stream_data(AudioData *audio_data, SDL_AudioStream *stream);



// log.c
void					print_audio_spec_info(SDL_AudioSpec micSpec, int micSample);
void					logExit(char *msg);
void					print_stream_format();
void					debug_mouse_state(Mouse_state mouse);

								/* window renderer surface font */
void					logger(void *w, void *r, void *s, void *f, const char *msg);

#endif
