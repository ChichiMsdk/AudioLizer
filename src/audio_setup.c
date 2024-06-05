#include "editor.h"

int
get_audio_capture_id(char *device_name)
{
    SDL_AudioDeviceID *adev;
	char *name;
	int device_id = 0;
	int device_count;

	if (!device_name)
		return SDL_AUDIO_DEVICE_DEFAULT_CAPTURE;

	if (!(adev = SDL_GetAudioCaptureDevices(&device_count)))
	{ fprintf(stderr, "Couldnt GetAudioCaptureDevices\n"); exit(1); }

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
		return SDL_AUDIO_DEVICE_DEFAULT_CAPTURE;

	return device_id;
}

int
get_audio_output_id(char *device_name)
{
    SDL_AudioDeviceID *adev;
	char *name;
	int device_id = 0;
	int device_count;

	/* if NULL use the default */
	if (!device_name)
		return SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;

	if (!(adev = SDL_GetAudioOutputDevices(&device_count)))
	{ fprintf(stderr, "Couldnt GetAudioOutputDevices\n"); exit(1); }

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
		return SDL_AUDIO_DEVICE_DEFAULT_OUTPUT;
	return device_id;
}

SDL_AudioStream*
stream_capture_init(SDL_AudioSpec a_spec, SDL_AudioDeviceID logical_dev_id)
{
	SDL_AudioStream *stream = SDL_CreateAudioStream(NULL, &a_spec);
	if (!stream)
		logExit("Failed to create audio stream");
	
	if (SDL_BindAudioStream(logical_dev_id, stream) == -1)
		logExit("Failed to bind stream");

	/* ensure no data is flowing when not needed (it auto starts at creation) */
	SDL_PauseAudioDevice(logical_dev_id);
	SDL_ClearAudioStream(stream);
	g_inst.stream = stream;
	return stream;
}

/* This should be called right before writing the file (for flength) */
void
wav_header_init(SDL_AudioSpec audio_spec)
{
	strcpy_s(g_wav_header.riff, 4, "RIFF");
	strcpy_s(g_wav_header.wave, 4,"WAVE");
	strcpy_s(g_wav_header.fmt, 4, "fmt ");
	strcpy_s(g_wav_header.data, 4, "data");

	g_wav_header.num_chans = audio_spec.channels;
	g_wav_header.bytes_per_samp = SDL_AUDIO_BYTESIZE(audio_spec.format);
	g_wav_header.bits_per_samp = SDL_AUDIO_BITSIZE(audio_spec.format);
	g_wav_header.bytes_per_sec = g_wav_header.srate * g_wav_header.bytes_per_samp;
	g_wav_header.srate = audio_spec.freq;
	g_wav_header.format_tag = 1;
	g_wav_header.chunk_size = 16;
	g_wav_header.flength = g_wav_header.dlength + 44; /* sizeof(wav_header) */
}

/* use this ??
 * SDL_OpenAudioDeviceStream (SDL_AudioDeviceID devid, const SDL_AudioSpec *spec
	 , SDL_AudioStreamCallback callback, void *userdata);
 */

SDL_AudioSpec
set_output_device(char *device_name)
{
	SDL_AudioSpec a_output_spec = {0};
	int sample = 0;

	// Audio_output_device specs
	int a_output_id = get_audio_output_id(device_name);
	if ((SDL_GetAudioDeviceFormat(a_output_id, &a_output_spec, &sample) < 0))
		logExit("Could not get GetAudioDeviceFormat");

	// Adapt them .. ?
	a_output_spec.format = SDL_AUDIO_S16LE;
	a_output_spec.channels = 1;
	a_output_spec.freq = 44100;
	print_audio_spec_info(a_output_spec, sample);

	SDL_AudioDeviceID logical_output_id;
    logical_output_id = SDL_OpenAudioDevice(a_output_id, 0);
	printf("Output: \"%s\"\n", SDL_GetAudioDeviceName(logical_output_id));

    if (!logical_output_id)
		logExit("Failed to open audio devices");

	g_inst.out_id = logical_output_id;
	return a_output_spec;
}

SDL_AudioSpec
set_capture_device(char *device_name)
{
	SDL_AudioSpec a_capture_spec = {0};
	int sample = 0;

	int a_capture_id = get_audio_capture_id(device_name);

	// Audio_capture_device specs
	if ((SDL_GetAudioDeviceFormat(a_capture_id, &a_capture_spec, &sample) < 0))
		logExit("Could not get GetAudioDeviceFormat");

	// Adapt them .. ?
	a_capture_spec.format = SDL_AUDIO_S16LE;
	a_capture_spec.channels = 1;
	a_capture_spec.freq = 44100;

	SDL_AudioDeviceID logical_capture_id;
    logical_capture_id = SDL_OpenAudioDevice(a_capture_id, &a_capture_spec);

    if (!logical_capture_id)
		logExit("Failed to open audio devices");

	printf("Capture: \"%s\"\n", SDL_GetAudioDeviceName(logical_capture_id));
	print_audio_spec_info(a_capture_spec, sample);


	g_inst.capture_id = logical_capture_id;
	return a_capture_spec;
}


void
print_stream_format()
{
	SDL_AudioSpec Ismp = {0};
	SDL_AudioSpec Osmp = {0};
	SDL_GetAudioStreamFormat(g_inst.stream, &Ismp, &Osmp);
	/*
	 * printf("-----------input stream format !!----------\n\n");
	 * print_mic_info(Ismp, 0);
	 * printf("-----------output stream format !!----------\n\n");
	 * print_mic_info(Osmp, 0);
	 */
}
