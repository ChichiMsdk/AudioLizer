#include "app.h"

void
stop(void *i)
{
	AudioData sfx = g_play_sfx;
	SDL_ClearAudioStream(sfx.stream);
	SDL_FlushAudioStream(sfx.stream);
	SDL_PauseAudioDevice(g_play_sfx.out_id);
	SDL_AudioStream *tmpst = g_play_sfx.stream;

	if (tmpst)
	{
		SDL_LockAudioStream(g_play_sfx.stream);
		g_play_sfx.stream = NULL;
		SDL_UnlockAudioStream(g_play_sfx.stream);
		SDL_DestroyAudioStream(tmpst);
	}
	SDL_free(g_play_sfx.buffer);
	g_play_sfx.buffer = NULL;
	g_play_sfx.length = 0;
	g_play_sfx.paused = true;
	g_play_sfx.reset = true;

}

void*
replay(void *i)
{
	AudioData sfx = g_play_sfx;
	SDL_ClearAudioStream(sfx.stream);
	SDL_FlushAudioStream(sfx.stream);
	g_play_sfx.reset = true;
	g_play_sfx.paused = false;
	SDL_ResumeAudioDevice(g_play_sfx.out_id);
	return NULL;
}

void*
my_toggle_play(void *sfx1)
{
	AudioData *sfx = (AudioData*)sfx1;
	if (SDL_AudioDevicePaused(sfx->out_id))
	{
		printf("Audio Playing\n");
		SDL_ResumeAudioDevice(sfx->out_id);
		sfx->paused = false;
	}
	else
	{
		printf("Audio Paused\n");
		SDL_PauseAudioDevice(sfx->out_id);
		sfx->paused = true;
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

/* destroys previous audio_data/stream playing */
AudioData
new_audio_to_play(const char *fname, int desired)
{
	SDL_PauseAudioDevice(g_play_sfx.out_id);
	SDL_AudioStream *tmpst = g_play_sfx.stream;

	if (tmpst)
	{
		SDL_LockAudioStream(g_play_sfx.stream);
		g_play_sfx.stream = NULL;
		SDL_UnlockAudioStream(g_play_sfx.stream);
		SDL_DestroyAudioStream(tmpst);
	}
	SDL_free(g_play_sfx.buffer);
	g_play_sfx.buffer = NULL;
	g_play_sfx.length = 0;
	g_play_sfx.path = fname;
	
	if (SDL_LoadWAV(g_play_sfx.path, &g_play_sfx.spec, &g_play_sfx.buffer, 
				&g_play_sfx.length))
	{ 
		fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); 
		return g_play_sfx;
	}
	/* printf("%s: %d kb\n", fname, g_play_sfx.length/1000); */

	/* exact amount of samples specified for one second */
	float samples = get_samples(g_play_sfx.spec);
	if (desired)
		samples = desired;

	g_play_sfx.samples = samples;
	tmpst = SDL_CreateAudioStream(&g_play_sfx.spec, &g_play_sfx.spec);
	if (!tmpst)
		logExit("CreateAudioStream Failed");

	if (SDL_BindAudioStream(g_play_sfx.out_id, tmpst) == -1)
		logExit("Failed to bind stream");

	SDL_PauseAudioDevice(g_play_sfx.out_id);
	g_play_sfx.paused = true;
	SDL_SetAudioStreamGetCallback(tmpst, put_callback, &g_play_sfx);

	SDL_LockAudioStream(tmpst);
	g_play_sfx.stream = tmpst;
	SDL_UnlockAudioStream(tmpst);
	return g_play_sfx;
}
