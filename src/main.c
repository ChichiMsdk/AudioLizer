#include			"app.h"
#include			"font.h"

#ifdef				WIN_32
	#include			<windows.h>
	#include			<io.h>
	LARGE_INTEGER		frequency;
	LARGE_INTEGER		start;
	LARGE_INTEGER		end;
	double				elpsd;
#endif

/* mandatory to launch app without console */
#define				SDL_MAIN_HANDLED
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
	int					buff_end = 0;
	float				g_volume = 1;
	void				*g_buffer = NULL;
	t_wav				g_wav_header = {0};
	AudioData			g_play_sfx = {0};
	char				text_input[BUFF_MAX];

void 
init_sdl(void)
{
#ifdef WIN_32
	QueryPerformanceFrequency(&frequency);
#endif
	/* is set for the capture device sample in set_capture_device */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{ fprintf(stderr, "Init video|audio: %s\n", SDL_GetError()); exit(1); }

	if (TTF_Init() != 0)
	{ fprintf(stderr, "%s\n", SDL_GetError()); exit(1); }

	g_inst.window = SDL_CreateWindow("Key capture", WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (g_inst.window == NULL)
	{ fprintf(stderr, "%s\n", SDL_GetError()); SDL_Quit(); exit(1); }

	g_inst.r = SDL_CreateRenderer(g_inst.window,NULL);
	if (g_inst.r == NULL)
		logExit("renderer failed to be created");
	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
}

Audio_wave
init_texture(void)
{
	Audio_wave wave = {.text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	wave.rect = (SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h};
	SDL_SetRenderTarget(g_inst.r, wave.text);
	SDL_SetRenderDrawColor(g_inst.r, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.r);
	return wave;
}

SDL_Texture*
init_svg(char const *arr, int w, int h)
{
	SDL_IOStream *fsvg = SDL_IOFromConstMem(arr, strlen(arr));
	if (!fsvg)
		logExit("Load svg from mem failed");
	SDL_Surface *ssvg = IMG_LoadSizedSVG_IO(fsvg, w, h);
	if (!ssvg)
		logExit("Load svg failed");
	SDL_Texture *text = SDL_CreateTextureFromSurface(g_inst.r, ssvg);
	if (!text)
		logExit("Load text from surface failed");
	SDL_DestroySurface(ssvg);
	if (SDL_CloseIO(fsvg))
		logExit("Could not close fsvg");
	return text;
}

/* 
 * note: add wrapper around timing functions os based 
 * note: add drag&drop files to play 
 */
int
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
		init_sdl();
		SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
		LogicalDevice dev_cap = {0}; LogicalDevice dev_out = {0}; AudioData cap_data = {0};
		init_audio_device(&dev_cap, cap_name, CAPTURE, spec);
		init_audio_stream(&dev_cap, dev_cap.spec, CAPTURE);
		g_inst.capture_id = dev_cap.logical_id;

		/* malloc c_data.buffer !! */
		cap_data = link_data_capture(dev_cap, dev_cap.stream, dev_cap.spec);
		g_inst.stream = cap_data.stream;

		/* should be done when saving */
		init_wav_header(&cap_data.header, cap_data.spec);

		SDL_SetRenderDrawBlendMode(g_inst.r, SDL_BLENDMODE_BLEND);
		Camera2D cam = init_camera(0, 0, 1.0f);
		g_inst.cam = &cam;
		Audio_wave wave = init_texture();

		init_audio_device(&dev_out, out_name, OUTPUT, g_play_sfx.spec);
		g_play_sfx.out_id = dev_out.logical_id;
		g_inst.out_id = dev_out.logical_id;
		g_inst.stream = g_play_sfx.stream;

		SDL_SetAudioStreamGetCallback(g_play_sfx.stream, put_callback, (void*)&g_play_sfx);

		g_inst.cursordefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
		if (!g_inst.cursordefault)
			logExit("Cursor failed");
		g_inst.cursorclick = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
		if (!g_inst.cursorclick)
			logExit("Cursor failed");
		SDL_SetCursor(g_inst.cursordefault);

	/* g_inst.buttons[0].text[0] = init_svg(svg, 100, 100); */
	init_button();
	memset(text_input, 0, BUFF_MAX);
	char *font_path = "E:\\Downloads\\installers\\4coder\\test_build\\fonts\\Inconsolata-Regular.ttf";
	TTF_Font *ttf = TTF_OpenFont(font_path, 64);
	font f;
	init_font(&f, g_inst.r, ttf);
	while (g_running)
	{
		Events(g_inst.e, &cap_data);
		SDL_SetRenderTarget(g_inst.r, NULL);
		SDL_SetRenderDrawColor(g_inst.r, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.r);

        	/*
        	 * if (g_retrieving == 0)
			 *  	retrieve_stream_data(&c_data, c_data.stream, 1);
			 * if (g_sending == 0)
			 * {
			 * 	render_wave(&wave, c_data.buffer, c_data.header.dlength);
			 * 	g_sending = 1;
			 * }
        	 */
		draw_buttons(g_inst.buttons);
		/* SDL_RenderTexture(g_inst.renderer, wave.text, NULL, &wave.rect); */
		SDL_RenderPresent(g_inst.r);
		/* boring */
		Sleep(4);
	}
	SDL_DestroyTexture(wave.text);
	/* too slow..  */
	/* SDL_free(sfx.buffer); */ 
	free(cap_data.buffer);
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

	SDL_DestroyCursor(g_inst.cursorclick);
	SDL_DestroyCursor(g_inst.cursordefault);
	SDL_DestroyAudioStream(g_play_sfx.stream);
	SDL_DestroyRenderer(g_inst.r);
	SDL_DestroyWindow(g_inst.window);
	TTF_Quit();
	SDL_Quit();
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
