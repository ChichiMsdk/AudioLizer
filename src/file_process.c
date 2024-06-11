#include "audio.h"

#define BUFF_SIZE 32768
/* #define BUFF_SIZE 1024 */

int					g_BUFF_SIZE = 1;
void				*g_buffer;

/*
 * void
 * vizualize_stream_data(AudioData *audio_data, SDL_AudioStream *stream)
 * {
 * 	size_t			bytes_read = 0;
 * 	size_t			bytes_available = 0;
 * 	static size_t	buffer_size = 0;
 * 	static size_t	allocated_size = FIRST_ALLOC;
 * 	static size_t	old_buff_size;
 * 	char			oldbuffer[1024 * g_BUFF_SIZE];
 * 	char			buffer[1024 * g_BUFF_SIZE];
 * 
 * 	SDL_FlushAudioStream(stream);
 * 
 * 	bytes_available = SDL_GetAudioStreamAvailable(stream);
 * 	#<{(| printf("bytes: %llu\n", bytes_available); |)}>#
 * 	if (bytes_available == 0) { return; }
 * 
 * 	bytes_read = SDL_GetAudioStreamData(stream, buffer+buffer_size,
 * 			audio_data->sample_size);
 * 
 * 	if (bytes_read == -1)
 * 	{ fprintf(stderr, "No bytes received from AudioStream..\n"); return ; }
 * 	#<{(| writes to sdl_rect and renderer |)}>#
 * 
 * 	buffer_size += bytes_read;
 * 	if (oldbuffer[0] == 0)
 * 		memcpy(oldbuffer, buffer, buffer_size);
 * 	if (old_buff_size == 0)
 * 		old_buff_size = buffer_size;
 * 	if (buffer_size >= 1024 * g_BUFF_SIZE)
 * 	{
 * 		#<{(| buffer_size = 1024 * g_BUFF_SIZE; |)}>#
 * 		plot_maker(buffer, buffer_size);
 * 		memcpy(oldbuffer, buffer, buffer_size);
 * 		old_buff_size = buffer_size;
 * 		buffer_size = 0;
 * 		SDL_ClearAudioStream(stream);
 * 	}
 * 	else
 * 		plot_maker(oldbuffer, old_buff_size);
 * }
 */

void
retrieve_stream_data(AudioData *audio_data, SDL_AudioStream *stream, int visu)
{
	size_t	bytes_read = 0;
	size_t	bytes_queued = 0;
	size_t	bytes_available = 0;
	int		buff_size = 1024 * g_BUFF_SIZE;

	assert(audio_data->sample_size > 0 && audio_data->sample_size <= BUFF_SIZE);

	/* idk if this should be static prolly not */
	static char buffer[BUFF_SIZE];
	memset(buffer, 0, audio_data->sample_size);

	bytes_queued = SDL_GetAudioStreamQueued(stream);
	bytes_available = SDL_GetAudioStreamAvailable(stream);
	if (bytes_available == 0) { return; }

	bytes_read = SDL_GetAudioStreamData(stream, buffer, audio_data->sample_size);

	if (bytes_read == -1)
	{ fprintf(stderr, "No bytes received from AudioStream..\n"); return ; }
	size_t tmp_length = audio_data->header.dlength + bytes_read;
	assert(tmp_length < MAX_BUFFER_SIZE);
	if (tmp_length >= audio_data->current_buff_size)
	{
		audio_data->current_buff_size *= 2;
		assert(audio_data->current_buff_size < MAX_BUFFER_SIZE);
		audio_data->buffer = realloc(audio_data->buffer, 
				audio_data->current_buff_size);

		assert(buffer != NULL);
	}
	memcpy((char*)audio_data->buffer + audio_data->header.dlength, buffer,
			bytes_read);
	memset(buffer, 0, bytes_read++);
	audio_data->header.dlength = tmp_length;
	if (visu == 0)
		make_realtime_plot(audio_data->buffer, audio_data->header.dlength);
}

void
adjust_volume_for_file(float factor, uint8_t *buffer, int32_t length)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;

	i = 0;
	data = (int16_t *)buffer;
	/* checks the number of samples; total size / size of 1 sample (2 byteshere) */
	number_samples= length / sizeof(int16_t);
	while (i < number_samples)
	{
		/* i dont really know whats the limit here for the volume ... */
		int32_t check_sample = (int32_t)(data[i] * factor);
		if (check_sample > INT16_MAX)
			check_sample = INT16_MAX;
		else if (check_sample < INT16_MIN)
			check_sample = INT16_MIN;
		data[i] = check_sample;
		i++;
	}
}

/* TODO: make appending file possible and volume choice in argv[] or ui */
void
save_file(char *file_name, AudioData *a_data)
{
	static size_t	bytes_written;
	float			volume_f = 50.0f;
	FILE			*file;

	printf("Writing to file...\n");
	file = fopen(file_name, "wb");
	if (!file) { perror("Error fopen line 151: "); exit(1); }

	/* adapt file length to new buffer length */
	a_data->header.flength = a_data->header.dlength + 44;
	bytes_written = fwrite(&a_data->header, sizeof(t_wav), 1, file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	/* volume is too low when recording for some reason .. */
	adjust_volume_for_file(volume_f, a_data->buffer, a_data->header.dlength);

	bytes_written += fwrite(a_data->buffer, 1, a_data->header.dlength, file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	fclose(file);

	printf("file_size is: %fKB\n", (double) a_data->header.flength/1000);
	printf("data_size is: %fKB\n", (double) a_data->header.dlength/1000);
}

AudioData
load_wav(const char *fpath)
{
	AudioData a_data = {};
	a_data.path = fpath;
	int error = 0;

	if (SDL_LoadWAV(a_data.path, &a_data.spec, &a_data.buffer, &a_data.length))
	{ printf("Error loading wav: %s\n", SDL_GetError()); exit(1); }

	return a_data;
}
