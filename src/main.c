#include			"app.h"

#ifdef				WIN_32
#include			<windows.h>
#include			<io.h>

LARGE_INTEGER		frequency;
LARGE_INTEGER		start;
LARGE_INTEGER		end;
double				elpsd;

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
AudioData			g_play_sfx = {0};

char const svg[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" height=\"32\" viewBox=\"0 0 48 48\"><path fill=\"currentColor\" d=\"M25.081 6.419C26.208 5.408 28 6.207 28 7.72v32.56c0 1.514-1.792 2.313-2.919 1.302l-8.206-7.366A4.75 4.75 0 0 0 13.702 33H9a5.25 5.25 0 0 1-5.25-5.25v-7.5C3.75 17.35 6.1 15 9 15h4.702a4.75 4.75 0 0 0 3.173-1.216zm.419 2.983l-6.955 6.244a7.25 7.25 0 0 1-4.843 1.854H9a2.75 2.75 0 0 0-2.75 2.75v7.5A2.75 2.75 0 0 0 9 30.5h4.702a7.25 7.25 0 0 1 4.843 1.855L25.5 38.6zm10.838-1.006a1.25 1.25 0 0 1 1.766-.058h.001l.01.01l.019.018a6 6 0 0 1 .262.267c.17.18.404.441.682.783a20.4 20.4 0 0 1 2.016 3.005C42.553 15.059 44 18.953 44 24s-1.447 8.94-2.906 11.58a20.4 20.4 0 0 1-2.016 3.004a15 15 0 0 1-.885.992l-.06.058l-.018.018l-.006.006l-.003.002v.001l.22-.22c.107-.107.143-.144-.222.22a1.25 1.25 0 0 1-1.71-1.822v-.001l.004-.003a5 5 0 0 0 .179-.183c.131-.14.326-.355.563-.647a18 18 0 0 0 1.766-2.635C40.198 32.034 41.5 28.553 41.5 24s-1.302-8.034-2.594-10.37a18 18 0 0 0-1.766-2.636a12 12 0 0 0-.71-.798l-.032-.032l-.003-.003l-.002-.002a1.25 1.25 0 0 1-.055-1.764M32.334 14.4a1.25 1.25 0 0 1 1.767-.065l.001.001l.002.002l.005.005l.014.012l.042.041q.051.05.137.139c.113.118.269.287.452.505c.366.436.847 1.072 1.326 1.893A14 14 0 0 1 38 24c0 3.023-.963 5.426-1.92 7.068c-.48.82-.96 1.457-1.326 1.893a10 10 0 0 1-.59.644l-.019.019l-.022.021l-.014.013l-.005.005l-.002.003H34.1a1.25 1.25 0 0 1-1.705-1.828l.002-.002l.016-.016l.085-.086c.078-.081.196-.209.34-.381c.29-.346.685-.866 1.081-1.545A11.5 11.5 0 0 0 35.5 24c0-2.477-.787-4.449-1.58-5.807c-.396-.68-.79-1.2-1.08-1.545a8 8 0 0 0-.426-.467l-.017-.017l.001.001a1.25 1.25 0 0 1-.064-1.765m5.781-6.052l-.01-.01l.001.002l.003.002zm-4.014 5.986l.064.062z\"/></svg>";

void 
init_sdl(void)
{
#ifdef WIN_32
	QueryPerformanceFrequency(&frequency);
#endif
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
	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
}

Audio_wave
init_text(void)
{
	Audio_wave wave = {.text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.renderer, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	wave.rect = (SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h};
	SDL_SetRenderTarget(g_inst.renderer, wave.text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);
	return wave;
}

SDL_Texture*
init_svg(char const *arr, int w, int h)
{
	SDL_IOStream *fsvg = SDL_IOFromConstMem(svg, strlen(svg));
	if (!fsvg)
		logExit("Load svg from mem failed");
	SDL_Surface *ssvg = IMG_LoadSizedSVG_IO(fsvg, w, h);
	if (!ssvg)
		logExit("Load svg failed");
	
	SDL_Texture *text = SDL_CreateTextureFromSurface(g_inst.renderer, ssvg);
	g_inst.buttons[0].text = text;
	if (!g_inst.buttons[0].text)
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
	init_button();

	SDL_SetRenderDrawBlendMode(g_inst.renderer, SDL_BLENDMODE_BLEND);
	Camera2D cam = init_camera(0, 0, 1.0f);
	g_inst.cam = &cam;
	Audio_wave wave = init_text();


	init_audio_device(&dev_out, out_name, OUTPUT, g_play_sfx.spec);
	g_play_sfx.out_id = dev_out.logical_id;
	g_inst.out_id = dev_out.logical_id;
	g_inst.stream = g_play_sfx.stream;

	SDL_SetAudioStreamGetCallback(g_play_sfx.stream, put_callback, (void*)&g_play_sfx);
	init_svg(svg, 100, 100);

	while (g_running)
	{
		Events(g_inst.e, &cap_data);
		SDL_SetRenderTarget(g_inst.renderer, NULL);
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
        
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
		SDL_RenderTexture(g_inst.renderer, wave.text, NULL, &wave.rect);
		SDL_RenderTexture(g_inst.renderer, g_inst.buttons[0].text, NULL, &(SDL_FRect){.x = 500, .y = 200, .w = 100, .h = 100});
		SDL_RenderPresent(g_inst.renderer);
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
	SDL_DestroyCursor(g_inst.cursor);
	SDL_DestroyAudioStream(g_play_sfx.stream);
	SDL_DestroyRenderer(g_inst.renderer);
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
