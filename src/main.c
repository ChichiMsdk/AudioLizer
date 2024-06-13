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
init_button()
{
	/* note: auto layout */
	g_inst.buttons = malloc(sizeof(Button) * 4);
	g_inst.buttons->count = 0;
	/* g_inst.button.rect = (SDL_FRect){ .x = 200.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f}; */
	g_inst.buttons[0].rect =(SDL_FRect){ .x = 200.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	g_inst.buttons[0].color = (SDL_Color) {.r = 170, .g = 50, .b = 50, .a = 255};
	g_inst.buttons[0].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	g_inst.buttons[0].hovered = false;
	g_inst.buttons[0].pressed = false;
	g_inst.buttons[0].released = false;
	g_inst.buttons[0].fn = test;

	g_inst.buttons[1].rect =(SDL_FRect){ .x = 350.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	g_inst.buttons[1].color = (SDL_Color) {.r = 170, .g = 50, .b = 0, .a = 255};
	g_inst.buttons[1].color_pressed = (SDL_Color) {.r = 100, .g = 150, .b = 50, .a = 255};
	g_inst.buttons[1].hovered = false;
	g_inst.buttons[1].pressed = false;
	g_inst.buttons[1].released = false;
	g_inst.buttons[1].fn = play_pause;

	g_inst.buttons[2].rect =(SDL_FRect){ .x = 500.0f, .y = 400.0f, .w = 100.0f, .h = 100.0f};
	g_inst.buttons[2].color = (SDL_Color) {.r = 170, .g = 100, .b = 50, .a = 255};
	g_inst.buttons[2].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	g_inst.buttons[2].hovered = false;
	g_inst.buttons[2].pressed = false;
	g_inst.buttons[2].released = false;
	g_inst.buttons[2].fn = replay;
	g_inst.buttons->count = 3;
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
	init_audio_device(&dev_capture, out_name, CAPTURE, spec);
	init_audio_stream(&dev_capture, dev_capture.spec, CAPTURE);
	g_inst.capture_id = dev_capture.logical_id;

	/* malloc c_data.buffer !! */
	AudioData c_data = link_data_capture(dev_capture, dev_capture.stream, dev_capture.spec);
	g_inst.stream = c_data.stream;

	/* should be done when saving */
	init_wav_header(&c_data.header, c_data.spec);
	init_button();
	SDL_SetRenderDrawBlendMode(g_inst.renderer, SDL_BLENDMODE_BLEND);

	Camera2D cam;
	init_camera(&cam, 0, 0, 1.0f);
	g_inst.cam = &cam;

	Audio_wave wave = { .text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.renderer, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	SDL_SetRenderTarget(g_inst.renderer, wave.text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);

	/* NOTE: make a "streamed" version, so not the whole file has to be loaded */
	AudioData sfx = {.path = "EE_VictoryMusic.wav"};
	if (SDL_LoadWAV(sfx.path, &sfx.spec, &sfx.buffer, &sfx.length))
		logExit("LoadWAV failed");

	sfx.stream = SDL_CreateAudioStream(&sfx.spec, &sfx.spec);
	if (sfx.stream == NULL)
		logExit("CreateAudioStream Failed");

	if (SDL_PutAudioStreamData(sfx.stream, sfx.buffer, sfx.length) < 0)
		logExit("PutAudioStream failed");

	init_audio_device(&dev_output, out_name, OUTPUT, sfx.spec);
	if (SDL_BindAudioStream(dev_output.logical_id, sfx.stream) == -1)
		logExit("Failed to bind stream");
	SDL_PauseAudioDevice(dev_output.logical_id);

	g_inst.out_id = dev_output.logical_id;
	g_inst.stream = sfx.stream;
	g_inst.sfx = sfx;

	load_full_wav("beethoven.wav");

	while (g_running)
	{
		SDL_SetRenderTarget(g_inst.renderer, NULL);
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
        
		if (g_retrieving == 0)
			retrieve_stream_data(&c_data, c_data.stream, 1);

		/* apply_camera(&cam, g_inst.renderer); */
		if (g_sending == 0)
		{
			render_wave(&wave, c_data.buffer, c_data.header.dlength);
			g_sending = 1;
		}
		Events(g_inst.e, &c_data);
		draw_buttons(g_inst.buttons);
		SDL_RenderTexture(g_inst.renderer, wave.text, NULL, 
				&(SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h});
		SDL_RenderPresent(g_inst.renderer);
	}
	SDL_DestroyTexture(wave.text);
	SDL_free(sfx.buffer);
	free(c_data.buffer);
	free(g_inst.buttons);
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
