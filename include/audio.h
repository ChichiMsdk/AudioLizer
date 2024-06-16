#ifndef AUDIO_H
#define AUDIO_H

#include "vector.h"

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_audio.h>

typedef enum 
{
	OUTPUT = 0,
	CAPTURE = 1
}DeviceType;

typedef struct wav_header
{
	char					riff[4];           /* "RIFF"                                  */
	int32_t					flength;           /* file length in bytes                    */
  	char					wave[4];           /* "WAVE"                                  */
  	char					fmt[4];            /* "fmt "                                  */
  	int32_t					chunk_size;        /* size of FMT chunk in bytes (usually 16) */
  	int16_t					format_tag;        /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
  	int16_t					num_chans;         /* 1=mono, 2=stereo                        */
  	int32_t					srate;             /* Sampling rate in samples per second     */
  	int32_t					bytes_per_sec;     /* bytes per second = srate*bytes_per_samp */
  	int16_t					bytes_per_samp;    /* 2=16-bit mono, 4=16-bit stereo          */
  	int16_t					bits_per_samp;     /* Number of bits per sample               */
  	char					data[4];           /* "data"                                  */
  	int32_t					dlength;           /* data length in bytes (filelength - 44)  */
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
	float				samples;
	float				duration;

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


// editor.c
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
AudioData				load_full_wav(const char *fpath);

#endif
