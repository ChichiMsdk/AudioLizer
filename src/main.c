#include "audio.h"

#include <SDL3/SDL_image.h>

/* mandatory to launch app without console */
#include <SDL3/SDL_main.h>

YUinstance			g_inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					g_retrieving = 1;
int					g_running = 1;
int					g_saving = 1;
void				*g_buffer;
t_wav				g_wav_header;

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

Mouse_state
get_mouse_state(void)
{
	float x, y;
	Mouse_state mouse;
	uint32_t flags = SDL_GetMouseState(&x, &y);
	mouse.pos = vec2f(x, y);
	mouse.flags = flags;
	return mouse;
}

AudioData
load_wav(const char *fpath)
{
	AudioData a_data = {};
	a_data.path = fpath;
	int error = 0;

	if (SDL_LoadWAV(a_data.path, &a_data.spec, &a_data.buffer, &a_data.length))
	{ printf("Error loading wav: %s\n", SDL_GetError()); exit(1); }

	return a_data;
}

void
vizualizer(AudioData a_data)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;
	i = 0;
	data = (int16_t *)a_data.buffer;
	/* checks the number of samples; total size / size of 1 sample (2 byteshere) */
	number_samples= g_wav_header.dlength / sizeof(int16_t);
	while (i < number_samples)
	{
		/* i dont really know whats the limit here for the volume ... */
		printf("data: %d\n", data[i]);
		i++;
	}
}

AudioData
link_data_capture(LogicalDevice device, SDL_AudioStream *stream,
		SDL_AudioSpec spec)
{
	AudioData a_data = {};
	a_data.spec = spec;
	a_data.sample_size = device.sample;
	a_data.stream = stream;
	if (!(a_data.buffer = malloc(FIRST_ALLOC)))
		logExit("malloc failed linking data");
	a_data.current_buff_size = FIRST_ALLOC;
	return a_data;
}

int
/* WinMain(int ac, char **av) */
main(int ac, char **av)
{
	g_inst.capture_name = NULL;
	g_inst.output_name = NULL;
	const char *cap_name = NULL;
	const char *out_name = NULL;

	/* gets potential value for devices */
	if (ac >= 2) { g_inst.capture_name = av[1]; cap_name = av[1];
		if (ac >=3) { g_inst.output_name = av[2]; out_name = av[2];}
	}

	init_sdl();

	/* we have to set it to zero or CONSEQUENCES if we want default */
	SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
	LogicalDevice dev_capture = {};
	LogicalDevice dev_output = {};
	init_audio_device(&dev_output, out_name, OUTPUT, spec);
	init_audio_device(&dev_capture, out_name, CAPTURE, spec);
	g_inst.out_id = dev_output.logical_id;
	g_inst.capture_id = dev_capture.logical_id;
	/* malloc c_data.buffer !! */
	AudioData c_data = 
		link_data_capture(dev_capture, dev_capture.stream, dev_capture.spec);

	init_wav_header(&c_data.header, c_data.spec);
	g_inst.stream = c_data.stream;
	AudioData a_data = load_wav("audio.wav");

	/* init_audio(); */
	/* Button button; */
	g_inst.button.rect = 
		(SDL_FRect){.x = 200.0f, .y = 150.0f, .w = 200.0f, .h = 100.0f};
	g_inst.button.hovered = 0;
	g_inst.button.pressed = 0; 
	g_inst.button.released = 0; 

	while (g_running)
	{
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
		/* wtf ? is this supposed to work like this ?..*/
		if (g_retrieving == 0)
			retrieve_stream_data(&c_data, c_data.stream);

        /*
		 * if (g_inst.button.hovered)
		 * {
		 * 	SDL_SetRenderDrawColor(g_inst.renderer, 100, 50, 50, 100);
		 * 	if (g_inst.button.pressed)
		 * 		SDL_SetRenderDrawColor(g_inst.renderer, 50, 170, 50, 100);
		 * }
		 * else
		 * 	SDL_SetRenderDrawColor(g_inst.renderer, 170, 50, 50, 255);
         * 
		 * SDL_RenderFillRect(g_inst.renderer, &g_inst.button.rect);
         */

		Events(g_inst.e, &c_data);

		SDL_RenderPresent(g_inst.renderer);
	}
	SDL_free(a_data.buffer);
	free(c_data.buffer);
	cleanup();
	return 0;
}

void
cleanup(void)
{
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
