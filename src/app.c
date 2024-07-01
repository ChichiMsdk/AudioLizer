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
	g_playlist.music[g_playlist.current].position = 0;
	g_playlist.paused = false;
	/* SDL_ResumeAudioDevice(g_playlist.out_id); */
	return NULL;
}

void*
playlist_next(void *i)
{
	/* SDL_LockMutex(g_playlist.mutex); */
	if (g_playlist.size <= 1)
	{
		/* SDL_UnlockMutex(g_playlist.mutex); */
		return NULL;
	}
	stop(NULL);
	int tmp = g_playlist.current;
	tmp++;
	assert(tmp < BUFF_MAX);
	if (tmp >= g_playlist.size)
		tmp = 0;
	change_audio_to_play(tmp, 0);
	/* SDL_UnlockMutex(g_playlist.mutex); */
	return i;
}

void*
playlist_back(void *i)
{
	/* SDL_LockMutex(g_playlist.mutex); */
	if (g_playlist.size <= 1)
	{
		/* SDL_UnlockMutex(g_playlist.mutex); */
		return NULL;
	}
	stop(NULL);
	int tmp = g_playlist.current;
	tmp--;
	if (tmp < 0)
		tmp = g_playlist.size - 1;
	change_audio_to_play(tmp, 0);
	/* SDL_UnlockMutex(g_playlist.mutex); */
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

int
get_samples(SDL_AudioSpec spec)
{ return (spec.freq * SDL_AUDIO_BYTESIZE(spec.format) * spec.channels); }

SDL_Texture *
create_static_text(TTF_Font *ttf, SDL_Renderer *r, const char *text);

static inline void
set_file_name_and_path(AudioData *audio, const char *file_path)
{
	size_t len = strlen(file_path);
	assert(len < MY_MAX_PATH);

	memcpy(audio->path, file_path, len);
	trim_file_name(audio->name, file_path);
	audio->texture = create_static_text(g_inst.ttf, g_inst.r, audio->name);
	if (!audio->texture)
		logExit("A texture for audio text could not be created");
	int w = 0, h = 0;
	if(TTF_SizeText(g_inst.ttf, audio->name, &w, &h) < 0)
		logExit("Could not get the size of the text");
	audio->rect.w = w;
	audio->rect.h = h;
}

static inline void
destroy_current_music_and_stream(SDL_AudioStream *s)
{
	if (s)
	{
		SDL_LockAudioStream(g_playlist.stream);
		g_playlist.stream = NULL;
		SDL_UnlockAudioStream(g_playlist.stream);
		SDL_DestroyAudioStream(s);
	}
	SDL_free(g_playlist.music[g_playlist.current].buffer);
	g_playlist.music[g_playlist.current].buffer = NULL;
	g_playlist.music[g_playlist.current].length = 0;
}

/* 
 * Ensures the device is paused after binding the stream
 */
static inline int
create_and_load_wav_stream(AudioData *audio, int desired, SDL_AudioStream **tmp_stream)
{
	if (SDL_LoadWAV(audio->path, &audio->spec, &audio->buffer, &audio->length))
	{ fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); return -1;}
	/* exact amount of samples specified for one second */
	int samples = get_samples(g_inst.out_dev.spec);
	if (desired)
		samples = desired;
	audio->samples = samples;
	if (audio->samples >= audio->length)
		audio->samples = audio->length;
	*tmp_stream = SDL_CreateAudioStream(&audio->spec, &audio->spec);
	if (!(*tmp_stream))
		logExit("CreateAudioStream Failed");
	if (SDL_BindAudioStream(g_playlist.out_id, *tmp_stream) == -1)
		logExit("Failed to bind stream");
	SDL_PauseAudioDevice(g_playlist.out_id);

	return 0;
}

void
change_audio_to_play(int index, int desired)
{
	AudioData audio = g_playlist.music[index];

	SDL_PauseAudioDevice(g_playlist.out_id);
	SDL_PauseAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = true;
	g_playlist.reset = true;
	SDL_AudioStream *tmp_stream = g_playlist.stream;

	destroy_current_music_and_stream(tmp_stream);
	create_and_load_wav_stream(&audio, desired, &tmp_stream);

	SDL_LockAudioStream(tmp_stream);
	g_playlist.current = index;
	g_playlist.stream = tmp_stream;
	int format = SDL_AUDIO_BYTESIZE(audio.spec.format);
	int channels = audio.spec.channels;
	audio.duration = audio.length / ((double) audio.spec.freq * format * channels);
	audio.position = 0;
	audio.remaining = audio.length;
	g_playlist.music[g_playlist.current] = audio;
	SDL_UnlockAudioStream(tmp_stream);

	SDL_ResumeAudioDevice(g_playlist.out_id);
	SDL_ResumeAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = false;
	SDL_SetAudioStreamGetCallback(tmp_stream, put_callback, &g_playlist);
	return ;
}

/* destroys previous audio_data/stream playing */
int
load_new_audio_to_play(const char *file_path, int desired, AudioData *a)
{
	AudioData audio = {0};

	SDL_PauseAudioDevice(g_playlist.out_id);
	SDL_PauseAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = true;
	g_playlist.reset = true;
	SDL_AudioStream *tmp_stream = g_playlist.stream;

	set_file_name_and_path(&audio, file_path);
	if (g_playlist.size > 0)
		destroy_current_music_and_stream(tmp_stream);
	if (create_and_load_wav_stream(&audio, desired, &tmp_stream) < 0)
		return -1;

	SDL_LockAudioStream(tmp_stream);
	g_playlist.stream = tmp_stream;
	g_playlist.current = g_playlist.size;
	assert(g_playlist.size < MAX_BUFFER_SIZE);
	int format = SDL_AUDIO_BYTESIZE(audio.spec.format);
	int channels = audio.spec.channels;
	audio.duration = audio.length / ((double) audio.spec.freq * format * channels);
	audio.position = 0;
	audio.remaining = audio.length;
	g_playlist.music[g_playlist.size] = audio;
	SDL_UnlockAudioStream(tmp_stream);

	g_playlist.size++;
	assert(g_playlist.size < MAX_BUFFER_SIZE);

	SDL_ResumeAudioDevice(g_playlist.out_id);
	SDL_ResumeAudioStreamDevice(g_playlist.stream);
	g_playlist.paused = false;
	SDL_SetAudioStreamGetCallback(tmp_stream, put_callback, &g_playlist);

	*a = audio;
	return 0;
}

	/*
	 * void
	 * add_new_audio(const char *fname, int desired)
	 * {
	 * 	AudioData audio = {0};
	 * 	size_t len = strlen(fname);
	 * 	assert(len < MY_MAX_PATH);
	 * 	memcpy(audio.path, fname, len);
	 * 	trim_file_name(audio.name, fname);
	 * 
	 * 	#<{(| check if it's a valid path |)}>#
	 * 	FILE *fd = fopen(fname, "r");
	 * 	if (!fd)
	 * 	{
	 * 		perror("Could not add audio: ");
	 * 		return ;
	 * 	}
	 * 	if (fclose(fd))
	 * 	{
	 * 		perror("Could not close file: ");
	 * 		return ;
	 * 	}
	 * 	memcpy(audio.path, fname, len);
	 * 	trim_file_name(audio.name, fname);
	 * 	#<{(| maybe if we can stream / use threads? |)}>#
	 * 	#<{(|
	 * 	 * if (SDL_LoadWAV(audio.path, &audio.spec, &audio.buffer, 
	 * 	 * 			&audio.length))
	 * 	 * { 
	 * 	 * 	fprintf(stderr, "Couldn't open dropped file: %s\n", SDL_GetError()); 
	 * 	 * 	return ;
	 * 	 * }
	 * 	 |)}>#
	 * 
	 * 	#<{(| exact amount of samples specified for one second |)}>#
	 * 
	 * 	assert(g_playlist.size < MAX_BUFFER_SIZE);
	 * 	g_playlist.music[g_playlist.size] = audio;
	 * 	g_playlist.size++;
	 * 	return ;
	 * }
	 * void*
	 * adjust_volume2(float factor, float *buf, int length)
	 * {
	 * 	size_t		i;
	 * 	size_t		number_samples;
	 * 
	 * 	#<{(| change = malloc(sizeof(uint16_t)*length); |)}>#
	 * 	i = 0;
	 * 	
	 * 	#<{(| checks the number of samples; total size / size of 1 sample (2 byteshere) |)}>#
	 * 	number_samples = length / sizeof(int16_t);
	 * 	while (i < number_samples)
	 * 	{
	 * 		#<{(| i dont really know whats the limit here for the volume ... |)}>#
	 * 		int32_t check_sample = (int32_t)(buf[i] * factor);
	 * 		if (check_sample > INT16_MAX)
	 * 			check_sample = INT16_MAX;
	 * 		else if (check_sample < INT16_MIN)
	 * 			check_sample = INT16_MIN;
	 * 		buf[i] = check_sample;
	 * 		i++;
	 * 	}
	 * 	return buf;
	 * 
	 * }
	 * 
	 * void*
	 * adjust_volume(float factor, void *buf, int length)
	 * {
	 * 	size_t		i;
	 * 	int16_t		*data;
	 * 	int16_t		*change;
	 * 	size_t		number_samples;
	 * 
	 * 	#<{(| change = malloc(sizeof(uint16_t)*length); |)}>#
	 * 	i = 0;
	 * 	data = (int16_t *)buf;
	 * 	#<{(| checks the number of samples; total size / size of 1 sample (2 byteshere) |)}>#
	 * 	number_samples = length / sizeof(int16_t);
	 * 	while (i < number_samples)
	 * 	{
	 * 		#<{(| i dont really know whats the limit here for the volume ... |)}>#
	 * 		int32_t check_sample = (int32_t)(data[i] * factor);
	 * 		if (check_sample > INT16_MAX)
	 * 			check_sample = INT16_MAX;
	 * 		else if (check_sample < INT16_MIN)
	 * 			check_sample = INT16_MIN;
	 * 		data[i] = check_sample;
	 * 		i++;
	 * 	}
	 * 	return data;
	 * }
	 */
