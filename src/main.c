#include			"app.h"

#ifdef				WIN_32
#include			<windows.h>
#include			<io.h>

LARGE_INTEGER		frequency;
LARGE_INTEGER		start;
LARGE_INTEGER		end;
double				elpsd;

#define				_CRT_SECURE_NO_WARNINGS
#define				WIN32_LEAN_AND_MEAN 
#endif

#define				SDL_MAIN_HANDLED

/* mandatory to launch app without console */
#include			<SDL3/SDL_main.h>

YUinstance			g_inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					g_retrieving = 1;
int					g_vizualizing = 1;
int					g_running = 1;
int					g_saving = 1;
int					g_sending = 1;
int					g_playing = 1;
float				g_volume = 1;
void				*g_buffer = NULL;
t_wav				g_wav_header = {0};
AudioData			*g_sfx = {0};

void *play_pause(void *id);

void 
init_sdl(void)
{
	/* is set for the capture device sample in set_capture_device */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{ fprintf(stderr, "%s\n", SDL_GetError()); exit(1); }

	g_inst.window = SDL_CreateWindow("Key capture", WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (g_inst.window == NULL)
	{ fprintf(stderr, "%s\n", SDL_GetError()); SDL_Quit(); exit(1); }

	g_inst.renderer = SDL_CreateRenderer(g_inst.window,NULL);
	if (g_inst.renderer == NULL)
		logExit("renderer failed to be created");
}

void
draw_buttons(Button *buttons)
{
	int i = 0;
	/* note: count of buttons*/
	while (i < buttons->count)
	{
		draw_button(buttons[i]);
		i++;
	}
}

void*
test(void *i)
{
	printf("Test\n");
	return NULL;
}

void*
replay(void *i)
{
	AudioData sfx = g_inst.sfx;
	if (SDL_PutAudioStreamData(sfx.stream, sfx.buffer, sfx.length) < 0)
		logExit("PutAudioStream failed");
	return NULL;
}

void
init_button(void)
{
	Button *b = {0};
	/* note: auto layout */
	b = malloc(sizeof(Button) * 4);
	b->count = 0;
	/* button.rect = (SDL_FRect){ .x = 200.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f}; */
	b[0].rect =(SDL_FRect){ .x = 200.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	b[0].color = (SDL_Color) {.r = 170, .g = 50, .b = 50, .a = 255};
	b[0].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	b[0].hovered = false;
	b[0].pressed = false;
	b[0].released = false;
	b[0].fn = test;

	b[1].rect =(SDL_FRect){ .x = 350.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	b[1].color = (SDL_Color) {.r = 170, .g = 50, .b = 0, .a = 255};
	b[1].color_pressed = (SDL_Color) {.r = 100, .g = 150, .b = 50, .a = 255};
	b[1].hovered = false;
	b[1].pressed = false;
	b[1].released = false;
	b[1].fn = play_pause;

	b[2].rect =(SDL_FRect){ .x = 500.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	b[2].color = (SDL_Color) {.r = 170, .g = 100, .b = 50, .a = 255};
	b[2].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	b[2].hovered = false;
	b[2].pressed = false;
	b[2].released = false;
	b[2].fn = replay;
	b->count = 3;
	g_inst.buttons = b;
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
	samples = desired;

	/*
	 * to make sure we have the right timing when adding more data 
	 * times 999 instead of 1000 to account for slowness and avoid cracklings
	 */
	float duration = ((float)samples / (float)sfx.spec.freq / 
			(float)SDL_AUDIO_BYTESIZE(sfx.spec.format) / (float)sfx.spec.channels) * 1000.0f;

	sfx.duration = duration;
	sfx.samples = samples;
	return sfx;
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

void SDLCALL
put_callback(void* usr, SDL_AudioStream *s, int add_amount, int total)
{
	AudioData sfx = *(AudioData*)usr;
	if (sfx.paused == true || !s || g_running == 0)
	{
		/* printf("sfx.paused = %d\n", sfx.paused); */
		return ;
	}

	float				d = sfx.duration;
	int					samples = sfx.samples;
	uint8_t				*buf = sfx.buffer;
	size_t				wav_length = sfx.length;
	size_t				offset;
	static uint64_t		count = -1;
	s = sfx.stream;
	if (count == -1)
		count++;
	offset = count * samples;
	/* sometimes fucks up ears if reaching very end*/
	if (offset >= wav_length - samples)
	{
		count = 0;
		offset = count * samples;
	}
	if (SDL_GetAudioStreamQueued(s) < samples)
	{
		count++;
		uint8_t *tmp = buf + offset;
		tmp = adjust_volume(g_volume, tmp, samples);
		if (SDL_PutAudioStreamData(s, tmp, samples) < 0)
			logExit("Couldnt put audio stream data\n");
		free(tmp);
		SDL_FlushAudioStream(s);
	}
	return ;
}

void
my_toggle_play(AudioData *sfx)
{
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
}

/* note: add wrapper around timing functions os based */
/* note: add drag&drop files to play */
int
/* WinMain(int ac, char **av) */
main(int ac, char **av)
{
	const char *cap_name = NULL;
	const char *out_name = NULL;
	if (ac >= 2) 
	{ 
		g_inst.capture_name = av[1]; cap_name = av[1];
		if (ac >=3)
		{ 
			g_inst.output_name = av[2]; out_name = av[2];
		}
	}
#ifdef WIN_32
	QueryPerformanceFrequency(&frequency);
#endif
	init_sdl();
	SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
	LogicalDevice dev_capture = {0};
	LogicalDevice dev_output = {0};
	AudioData capture_data = {0};
	init_audio_device(&dev_capture, out_name, CAPTURE, spec);
	init_audio_stream(&dev_capture, dev_capture.spec, CAPTURE);
	g_inst.capture_id = dev_capture.logical_id;

	/* malloc c_data.buffer !! */
	capture_data = link_data_capture(dev_capture, dev_capture.stream, dev_capture.spec);
	g_inst.stream = capture_data.stream;

	/* should be done when saving */
	init_wav_header(&capture_data.header, capture_data.spec);
	init_button();
	SDL_SetRenderDrawBlendMode(g_inst.renderer, SDL_BLENDMODE_BLEND);

	Camera2D cam;
	init_camera(&cam, 0, 0, 1.0f);
	g_inst.cam = &cam;

	Audio_wave wave = {
		.text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4, .current = 0};

	wave.text = SDL_CreateTexture(g_inst.renderer, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	SDL_SetRenderTarget(g_inst.renderer, wave.text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);

	AudioData sfx = init_audio_to_play("Beethoven.wav", 4096);

	sfx.stream = SDL_CreateAudioStream(&sfx.spec, &sfx.spec);
	if (sfx.stream == NULL)
		logExit("CreateAudioStream Failed");

	if (SDL_PutAudioStreamData(sfx.stream, sfx.buffer, sfx.length) < 0)
		logExit("PutAudioStream failed");

	init_audio_device(&dev_output, out_name, OUTPUT, sfx.spec);
	if (SDL_BindAudioStream(dev_output.logical_id, sfx.stream) == -1)
		logExit("Failed to bind stream");
	SDL_PauseAudioDevice(dev_output.logical_id);
	sfx.paused = true;

	g_inst.out_id = dev_output.logical_id;
	g_inst.stream = sfx.stream;
	g_inst.sfx = sfx;
	sfx.out_id = dev_output.logical_id;
	g_sfx = &sfx;

	/* load_full_wav("beethoven.wav"); */
	SDL_SetAudioStreamGetCallback(g_sfx->stream, put_callback, (void*)&sfx);
	while (g_running)
	{
		SDL_SetRenderTarget(g_inst.renderer, NULL);
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
        
		/* if (g_sending == 0) */
			/* add_more_audio(sfx); */
        /*
		 * if (g_retrieving == 0)
		 * 	retrieve_stream_data(&c_data, c_data.stream, 1);
         */

		if (g_sending == 0)
		{
			/* render_wave(&wave, c_data.buffer, c_data.header.dlength); */
			/* g_sending = 1; */
		}
		Events(g_inst.e, &capture_data);
		draw_buttons(g_inst.buttons);
		SDL_RenderTexture(g_inst.renderer, wave.text, NULL, 
				&(SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h});
		SDL_RenderPresent(g_inst.renderer);
	}
	SDL_DestroyTexture(wave.text);
	SDL_free(sfx.buffer);
	free(capture_data.buffer);
	free(g_inst.buttons);
	cleanup();
	return 0;
}

void
cleanup(void)
{
	/* SDL_DestroyTexture(g_inst.texture); */

	/* fucking slow these two */
	SDL_CloseAudioDevice(g_inst.out_id);
	SDL_CloseAudioDevice(g_inst.capture_id);
	SDL_DestroyCursor(g_inst.cursor);
	SDL_DestroyAudioStream(g_inst.stream);
	SDL_DestroyRenderer(g_inst.renderer);
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
