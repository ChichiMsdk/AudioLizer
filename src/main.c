#include "app.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#define SDL_MAIN_HANDLED

/* mandatory to launch app without console */
#include <SDL3/SDL_main.h>

YUinstance			g_inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					g_retrieving = 1;
int					g_vizualizing = 1;
int					g_running = 1;
int					g_saving = 1;
int					g_sending = 1;
int					g_playing = 1;
void				*g_buffer = NULL;
t_wav				g_wav_header = {};

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
	init_sdl();
	SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
	LogicalDevice dev_capture = {};
	LogicalDevice dev_output = {};
	init_audio_device(&dev_output, out_name, OUTPUT, spec);
	init_audio_device(&dev_capture, out_name, CAPTURE, spec);
	g_inst.out_id = dev_output.logical_id;
	g_inst.capture_id = dev_capture.logical_id;
	/* malloc c_data.buffer !! */
	AudioData c_data = link_data_capture(dev_capture, dev_capture.stream, dev_capture.spec);
	g_inst.stream = c_data.stream;

	/* should be done when saving */
	init_wav_header(&c_data.header, c_data.spec);

	g_inst.button.rect = (SDL_FRect){ .x = 200.0f, .y = 400.0f, .w = 200.0f, .h = 100.0f};

	Camera2D cam;
	init_camera(&cam, 0, 0, 1.0f);
	g_inst.cam = &cam;

	Audio_wave wave = { .text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4};
	wave.text = SDL_CreateTexture(g_inst.renderer, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	SDL_SetRenderTarget(g_inst.renderer, wave.text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);

	/* NOTE: make a "streamed" version, so not the whole file has to be loaded */
	AudioData sfx = {.path = "EE_VictoryMusic.wav"};
	if (SDL_LoadWAV(sfx.path, &sfx.spec, &sfx.buffer, &sfx.length))
		logExit("LoadWAV failed");

	while (g_running)
	{
		SDL_SetRenderTarget(g_inst.renderer, NULL);
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
        
		if (g_retrieving == 0)
			retrieve_stream_data(&c_data, c_data.stream, g_vizualizing);

		/* apply_camera(&cam, g_inst.renderer); */
		if (g_sending == 0)
		{
			render_wave(&wave, c_data.buffer, c_data.header.dlength);
			g_sending = 1;
		}
		Events(g_inst.e, &c_data);
		draw_button();
		SDL_RenderTexture(g_inst.renderer, wave.text, NULL, 
				&(SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h});
		if (g_playing == 0)
		{
			load_to_stream(&sfx);
		}

		SDL_RenderPresent(g_inst.renderer);
	}
	SDL_DestroyTexture(wave.text);
	SDL_free(sfx.buffer);
	free(c_data.buffer);
	cleanup();
	return 0;
}

void
cleanup(void)
{
	/* SDL_DestroyTexture(g_inst.texture); */
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
