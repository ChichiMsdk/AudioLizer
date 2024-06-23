#include "app.h"

void*
stop(void *i)
{
	SDL_ClearAudioStream(g_playlist.stream);
	SDL_FlushAudioStream(g_playlist.stream);
	SDL_PauseAudioDevice(g_playlist.out_id);
	SDL_AudioStream *tmpst = g_playlist.stream;

	if (tmpst)
	{
		SDL_LockAudioStream(g_playlist.stream);
		g_playlist.stream = NULL;
		SDL_UnlockAudioStream(g_playlist.stream);
		SDL_DestroyAudioStream(tmpst);
	}
	SDL_free(g_playlist.music[g_playlist.current].buffer);
	g_playlist.music[g_playlist.current].buffer = NULL;
	g_playlist.music[g_playlist.current].length = 0;
	g_playlist.paused = true;
	g_playlist.reset = true;
	return NULL;
}

void*
replay(void *i)
{
	SDL_ClearAudioStream(g_playlist.stream);
	SDL_FlushAudioStream(g_playlist.stream);
	g_playlist.reset = true;
	g_playlist.paused = false;
	SDL_ResumeAudioDevice(g_playlist.out_id);
	return NULL;
}

void*
playlist_next(void *i)
{
	return i;
}

void*
my_toggle_play(void *sfx1)
{
	if (SDL_AudioDevicePaused(g_playlist.out_id))
	{
		printf("Audio Playing\n");
		SDL_ResumeAudioDevice(g_playlist.out_id);
		g_playlist.paused = false;
	}
	else
	{
		printf("Audio Paused\n");
		SDL_PauseAudioDevice(g_playlist.out_id);
		g_playlist.paused = true;
	}
	return sfx1;
}

uint8_t*
adjust_volume(float factor, uint8_t *buf, int length)
{
	size_t		i;
	int16_t		*data;
	int16_t		*change = malloc(sizeof(uint16_t)*length);
	size_t		number_samples;

	i = 0;
	data = (int16_t *)buf;
	/* checks the number of samples; total size / size of 1 sample (2 byteshere) */
	number_samples = length / sizeof(int16_t);
	while (i < number_samples)
	{
		/* i dont really know whats the limit here for the volume ... */
		int32_t check_sample = (int32_t)(data[i] * factor);
		if (check_sample > INT16_MAX)
			check_sample = INT16_MAX;
		else if (check_sample < INT16_MIN)
			check_sample = INT16_MIN;
		change[i] = check_sample;
		i++;
	}
	return (uint8_t *)change;
}

float
get_samples(SDL_AudioSpec spec)
{
	return (float)(spec.freq * SDL_AUDIO_BYTESIZE(spec.format) * spec.channels);
}

void
change_audio_to_play(int index, int desired)
{
	SDL_PauseAudioDevice(g_playlist.out_id);
	SDL_PauseAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = true;
	g_playlist.reset = true;
	SDL_AudioStream *tmpst = g_playlist.stream;
	AudioData audio = g_playlist.music[index];

	if (tmpst)
	{
		SDL_LockAudioStream(g_playlist.stream);
		g_playlist.stream = NULL;
		SDL_UnlockAudioStream(g_playlist.stream);
		SDL_DestroyAudioStream(tmpst);
	}
	SDL_free(g_playlist.music[g_playlist.current].buffer);
	g_playlist.music[g_playlist.current].buffer = NULL;
	g_playlist.music[g_playlist.current].length = 0;

	if (SDL_LoadWAV(audio.path, &audio.spec, &audio.buffer, &audio.length))
	{ 
		fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); 
		return ;
	}

	g_playlist.current = index;
	/* exact amount of samples specified for one second */
	float samples = get_samples(audio.spec);
	if (desired)
		samples = desired;

	audio.samples = samples;
	tmpst = SDL_CreateAudioStream(&audio.spec, &audio.spec);
	if (!tmpst)
		logExit("CreateAudioStream Failed");

	if (SDL_BindAudioStream(g_playlist.out_id, tmpst) == -1)
		logExit("Failed to bind stream");

	SDL_PauseAudioDevice(g_playlist.out_id);

	SDL_LockAudioStream(tmpst);
	g_playlist.stream = tmpst;
	SDL_UnlockAudioStream(tmpst);
	SDL_SetAudioStreamGetCallback(tmpst, put_callback, &g_playlist);
	SDL_ResumeAudioDevice(g_playlist.out_id);
	SDL_ResumeAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = false;
	return ;
}

void
add_new_audio(const char *fname, int desired)
{
	AudioData audio = {0};
	size_t len = strlen(fname);
	assert(len < MY_MAX_PATH);
	memcpy(audio.path, fname, len);
	trim_file_name(audio.name, fname);

	/* check if it's a valid path */
	FILE *fd = fopen(fname, "r");
	if (!fd)
	{
		perror("Could not add audio: ");
		return ;
	}
	if (fclose(fd))
	{
		perror("Could not close file: ");
		return ;
	}
	memcpy(audio.path, fname, len);
	trim_file_name(audio.name, fname);
	/* maybe if we can stream / use threads? */
    /*
	 * if (SDL_LoadWAV(audio.path, &audio.spec, &audio.buffer, 
	 * 			&audio.length))
	 * { 
	 * 	fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); 
	 * 	return ;
	 * }
     */

	/* exact amount of samples specified for one second */

	assert(g_playlist.size < MAX_BUFFER_SIZE);
	g_playlist.music[g_playlist.size] = audio;
	g_playlist.size++;
	return ;
}

/* destroys previous audio_data/stream playing */
AudioData
load_new_audio_to_play(const char *fname, int desired)
{
	SDL_PauseAudioDevice(g_playlist.out_id);
	SDL_PauseAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = true;
	g_playlist.reset = true;
	SDL_AudioStream *tmpst = g_playlist.stream;
	AudioData audio = {0};
	size_t len = strlen(fname);
	assert(len < MY_MAX_PATH);
	memcpy(audio.path, fname, len);
	trim_file_name(audio.name, fname);
	/* printf("path: %s\n", audio.path); */
	if (g_playlist.size == 0)
		goto skip;
	if (tmpst)
	{
		SDL_LockAudioStream(g_playlist.stream);
		g_playlist.stream = NULL;
		SDL_UnlockAudioStream(g_playlist.stream);
		SDL_DestroyAudioStream(tmpst);
	}
	SDL_free(g_playlist.music[g_playlist.current].buffer);
	g_playlist.music[g_playlist.current].buffer = NULL;
	g_playlist.music[g_playlist.current].length = 0;
	
skip:
	if (SDL_LoadWAV(audio.path, &audio.spec, &audio.buffer, 
				&audio.length))
	{ 
		fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); 
		return audio;
	}
	/* printf("%s: %d kb\n", fname, g_playlist.length/1000); */

	/* exact amount of samples specified for one second */
	float samples = get_samples(audio.spec);
	if (desired)
		samples = desired;

	audio.samples = samples;
	tmpst = SDL_CreateAudioStream(&audio.spec, &audio.spec);
	if (!tmpst)
		logExit("CreateAudioStream Failed");

	if (SDL_BindAudioStream(g_playlist.out_id, tmpst) == -1)
		logExit("Failed to bind stream");

	SDL_PauseAudioDevice(g_playlist.out_id);

	SDL_LockAudioStream(tmpst);
	g_playlist.stream = tmpst;
	SDL_UnlockAudioStream(tmpst);
	assert(g_playlist.size < MAX_BUFFER_SIZE);
	g_playlist.music[g_playlist.size] = audio;
	g_playlist.current = g_playlist.size;
	g_playlist.size++;
	SDL_SetAudioStreamGetCallback(tmpst, put_callback, &g_playlist);
	SDL_ResumeAudioDevice(g_playlist.out_id);
	SDL_ResumeAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = false;
	return audio;
}
