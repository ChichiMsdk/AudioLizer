#include "audio.h"

/* change error handling */
SDL_AudioStream*
init_audio_stream(LogicalDevice *device, SDL_AudioSpec spec, DeviceType type)
{
	SDL_AudioStream *stream = SDL_CreateAudioStream(NULL, &spec);
	if (!stream)
		logExit("Failed to create audio stream");
	if (SDL_BindAudioStream(device->logical_id, stream) == -1)
		logExit("Failed to bind stream");
	if (type == CAPTURE)
	{
		SDL_PauseAudioDevice(device->logical_id);
		SDL_ClearAudioStream(stream);
	}
	device->stream = stream;
	return stream;
}

void
init_wav_header(t_wav *header, SDL_AudioSpec audio_spec)
{
	/* fkn microsoft strcpy_s forces the '\0' implicitely -_- */
	strncpy(header->riff, "RIFF", 4);
	strncpy(header->wave, "WAVE", 4);
	strncpy(header->fmt, "fmt ", 4);
	strncpy(header->data, "data", 4);

	header->num_chans = audio_spec.channels;
	header->bytes_per_samp = SDL_AUDIO_BYTESIZE(audio_spec.format);
	header->bits_per_samp = SDL_AUDIO_BITSIZE(audio_spec.format);
	header->bytes_per_sec =
		header->srate * header->bytes_per_samp;

	header->srate = audio_spec.freq;
	header->format_tag = 1;
	header->chunk_size = 16;
	header->dlength = 0;
	header->flength = 0;
	/* header->flength = 
	   header->dlength + 44; // sizeof(wav_header) */
}

int
get_audio_device_id(const char *device_name, DeviceType type)
{
    SDL_AudioDeviceID *adev;
	char *name;
	int device_id = 0;
	int device_count;

	if (!device_name)
	{
		if (type == OUTPUT) return SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;
		else return SDL_AUDIO_DEVICE_DEFAULT_CAPTURE;
	}

	if (type == CAPTURE)
	{
		if (!(adev = SDL_GetAudioCaptureDevices(&device_count)))
		{ fprintf(stderr, "Couldnt GetAudioCaptureDevices\n"); exit(1); }
	}
	else
	{
		if (!(adev = SDL_GetAudioOutputDevices(&device_count)))
		{ fprintf(stderr, "Couldnt GetAudioOutputDevices\n"); exit(1); }
	}

	void *temp = adev;
	while (*adev)
	{
		name = SDL_GetAudioDeviceName(*adev);
		if (name && strstr(name, device_name))
		{
			/* printf("\n\nDevice capture %d: %s\n\n", *adev, name); */
			device_id = *adev;
		}
		adev++;
	}
	SDL_free(temp);

	if (device_id == 0)
	{
		if (type == OUTPUT) return SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;
		else return SDL_AUDIO_DEVICE_DEFAULT_CAPTURE;
	}

	return device_id;
}

SDL_AudioSpec
set_audio_device(LogicalDevice *device)
{
	char *type = NULL;

	if (device->type == CAPTURE)
		type = "Capture";
	else
		type = "Output";

	SDL_AudioSpec spec = {};
	device->physical_id = get_audio_device_id(device->name, device->type);
	// Audio_device specs
	if ((SDL_GetAudioDeviceFormat(
					device->physical_id, &spec, &device->sample) < 0))
	{
		logExit("Could not get GetAudioDeviceFormat");
	}

	/* channels should therefore never be == 0 if valid spec... */
	if (device->spec.channels == 0)
	{
		/* means we asked for default config */
		device->spec = spec;
	}

    device->logical_id = 
		SDL_OpenAudioDevice(device->physical_id, &device->spec);

    if (!device->logical_id)
		logExit("Failed to open audio devices");

	printf("%s: \"%s\"\n", type, SDL_GetAudioDeviceName(device->logical_id));
	print_audio_spec_info(device->spec, device->sample);

	return device->spec;
}

void
init_audio_device(LogicalDevice *device, const char *name, DeviceType type,
		SDL_AudioSpec spec)
{
	/* make tag memory to be sure we free it */
    /*
	 * device->buffer = malloc(FIRST_ALLOC); assert(device->buffer);
	 * device->current_buff_size = FIRST_ALLOC;
     */
	device->name = name;
	device->type = type;
	device->spec = spec;

	set_audio_device(device);
	init_audio_stream(device, device->spec, device->type);
}

#define BUFF_SIZE 4096

void
retrieve_stream_data2(AudioData *audio_data, SDL_AudioStream *stream)
{
	size_t	bytes_read = 0;
	size_t	bytes_queued = 0;
	size_t	bytes_available = 0;
	/* int		buff_size = 4096; */

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
}

void
adjust_volume_for_file2(float factor, uint8_t *buffer, int32_t length)
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
save_file2(char *file_name, AudioData *a_data)
{
	static size_t	bytes_written;
	float			volume_f = 50.0f;
	FILE			*file;

	file = fopen(file_name, "wb");
	if (!file) { perror("Error fopen line 151: "); exit(1); }

	/* adapt file length to new buffer length */
	a_data->header.flength = a_data->header.dlength + 44;
	bytes_written = fwrite(&a_data->header, sizeof(t_wav), 1, file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	/* volume is too low when recording for some reason .. */
	adjust_volume_for_file2(volume_f, a_data->buffer, a_data->header.dlength);

	bytes_written += fwrite(a_data->buffer, 1, a_data->header.dlength, file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	fclose(file);

	printf("file_size is: %fKB\n", (double) a_data->header.flength/1000);
	printf("data_size is: %fKB\n", (double) a_data->header.dlength/1000);
}
