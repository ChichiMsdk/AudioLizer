#include "app.h"
#include <io.h>

#define BUFF_SIZE 32768
/* #define BUFF_SIZE 1024 */

int					g_BUFF_SIZE = 1;
void				*g_buffer;

void
retrieve_stream_data(AudioData *audio_data, SDL_AudioStream *stream, int visu)
{
	size_t	bytes_read = 0;
	size_t	bytes_queued = 0;
	size_t	bytes_available = 0;
	int		buff_size = 32768;

	audio_data->sample_size = audio_data->samples;
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

	SDL_FlushAudioStream(a_data->stream);
	while(SDL_GetAudioStreamAvailable(a_data->stream))
		retrieve_stream_data(a_data, a_data->stream, 1);
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

void
print_wav_header(t_wav header)
{
	_write(1, "\n", 1);
	_write(1, (char *)header.riff, 4); _write(1, "|\n", 2);
	printf("flength: %d\n", header.flength);
	_write(1, (char *)header.wave, 4); _write(1, "|\n", 2);
	_write(1, (char *)header.fmt, 4); _write(1, "|\n", 2);
	printf("chunk_size: %d\n", header.chunk_size);
	printf("format_tag: %d\n", header.format_tag);
	printf("num_chans: %d\n", header.num_chans);
	printf("srate: %d\n", header.srate);
	printf("bytes_per_sec: %d\n", header.bytes_per_sec);
	printf("bytes_per_samp: %d\n", header.bytes_per_samp);
	printf("bits_per_samp: %d\n", header.bits_per_samp);
	_write(1, (char *)header.data, 4); _write(1, "|\n", 2);
	printf("dlength: %d\n", header.dlength);
}

AudioData
load_full_wav(const char *fpath)
{
	AudioData a_data = {};
	t_wav header = {};
	size_t real_size = 0;
	size_t total_size = 0;

	a_data.path = fpath;
	int error = 0;

	FILE *fd = fopen(fpath, "rb");
	if (fd == NULL)
		logExit("fopen failed");

	size_t count = fread(&header, sizeof(t_wav), 1, fd);
	if (count < 0)
		logExit("fread failed");
	a_data.header = header;
	/* print_wav_header(header); */

	/* dont trust riff header since it can provide wrong size... */
	int offset = ftell(fd);

	fseek(fd, (offset * -1), SEEK_END);
	real_size = ftell(fd);
	fseek(fd, 0L, SEEK_END);
	total_size = ftell(fd);
	fseek(fd, offset, SEEK_SET);

	void *buffer = malloc(real_size);
	if (!buffer)
		logExit("malloc failed");

	count = fread(buffer, real_size, 1, fd);
	if (count < 0)
		logExit("fopen failed");

	printf("-----\n\ncount: %llu\n", count);
	printf("real: %llu\n", real_size);
	printf("total: %llu\n", total_size);
	printf("dlength: %d\n", header.dlength);
	printf("flength: %d\n", header.flength);
	fclose(fd);
	free(buffer);
	exit(1);

    /*
	 * if (SDL_LoadWAV(a_data.path, &a_data.spec, &a_data.buffer, &a_data.length))
	 * { printf("Error loading wav: %s\n", SDL_GetError()); exit(1); }
     */

	return a_data;
}

AudioData
init_audio_to_play(const char *f_name, int desired)
{
	/* note: make a "streamed" version, so not the whole file has to be loaded */
	AudioData sfx = {.path = f_name};
	if (SDL_LoadWAV(sfx.path, &sfx.spec, &sfx.buffer, &sfx.length))
		logExit("LoadWAV failed");
	/*
	 * this is the exact amount of samples specified by the file for 
	 * one second exactly 
	 */
	float samples = (float)sfx.spec.freq * (float)SDL_AUDIO_BYTESIZE(sfx.spec.format)
					* (float)sfx.spec.channels;
	/* desired needs to be > 0 to not default to samples */
	if (desired)
		samples = desired;

	/*
	 * to make sure we have the right timing when adding more data 
	 * times 999 instead of 1000 to account for slowness and avoid cracklings
	 */
	float duration = ((float)samples / (float)sfx.spec.freq / 
			(float)SDL_AUDIO_BYTESIZE(sfx.spec.format) / (float)sfx.spec.channels) * 1000.0f;

	sfx.duration = duration;
	sfx.samples = samples;
	sfx.stream = SDL_CreateAudioStream(&sfx.spec, &sfx.spec);
	if (sfx.stream == NULL)
		logExit("CreateAudioStream Failed");
	return sfx;
}

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
