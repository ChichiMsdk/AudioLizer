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
AudioData			g_play_sfx = {0};

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
