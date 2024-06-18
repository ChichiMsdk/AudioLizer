#include "app.h"

/* change error handling */
SDL_AudioStream*
init_audio_stream(LogicalDevice *device, SDL_AudioSpec spec, DeviceType type)
{
	SDL_AudioStream *stream = SDL_CreateAudioStream(NULL, &spec);
	if (!stream)
		logExit("Failed to create audio stream");
	if (SDL_BindAudioStream(device->logical_id, stream) == -1)
		logExit("Failed to bind stream");

	SDL_PauseAudioDevice(device->logical_id);
	if (type == CAPTURE)
	{
		SDL_ClearAudioStream(stream);
	}
	device->stream = stream;
	return stream;
}

/* This should be called right before writing the file (for flength) */
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
	const char *name;
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

/* use this ??
 * SDL_OpenAudioDeviceStream(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec,
 * 		SDL_AudioStreamCallback callback, void *userdata);
 */

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
	/* init_audio_stream(device, device->spec, device->type); */
}

AudioData
link_data_capture(LogicalDevice device, SDL_AudioStream *stream,
		SDL_AudioSpec spec)
{
	AudioData a_data = {};
	a_data.spec = spec;
	a_data.sample_size = device.sample;
	a_data.stream = stream;
	if (!(a_data.buffer = malloc(FIRST_ALLOC)))
		logExit("malloc failed linking data");
	a_data.current_buff_size = FIRST_ALLOC;
	return a_data;
}

void SDLCALL
put_callback(void* usr, SDL_AudioStream *s, int add_amount, int total)
{
	AudioData sfx = *(AudioData*)usr;
	/* AudioData sfx = g_sfx; */
	if (!s)
	{
		printf("s is null\n");
		return ;
	}
	if (g_running == 0)
	{
		printf("running is zero\n");
		return ;
	}
	float				d = sfx.duration;
	int					samples = sfx.samples;
	uint8_t				*buf = sfx.buffer;
	size_t				wav_length = sfx.length;
	size_t				offset;
	static uint64_t		count = 0;
	s = sfx.stream;
	if (g_play_sfx.reset == true)
	{
		count = 0;
		g_play_sfx.reset = false;
	}
	offset = count * samples;
	/* sometimes fucks up ears if reaching very end*/
	if (offset >= wav_length - samples)
	{
		if (offset >= wav_length)
		{
			count = 0;
			offset = count * samples;
		}
		else
			samples = wav_length - offset - 2;
	}
	if (SDL_GetAudioStreamQueued(s) < samples)
	{
		count++;
		uint8_t *tmp = buf + offset;
		tmp = adjust_volume(g_volume, tmp, samples);
		if (SDL_PutAudioStreamData(s, tmp, samples) < 0)
			logExit("Couldnt put audio stream data in callback\n");
		free(tmp);
		SDL_FlushAudioStream(s);
	}
	return ;
}
