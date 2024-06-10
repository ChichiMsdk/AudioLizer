#include "audio.h"

/* #include <SDL3/SDL_image.h> */

/* mandatory to launch app without console */
#include <SDL3/SDL_main.h>

YUinstance			g_inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					g_retrieving = 1;
int					g_vizualizing = 1;
int					g_running = 1;
int					g_saving = 1;
void				*g_buffer;
t_wav				g_wav_header;

typedef struct pub
{
	int x1, x2, y1, y2;
}pub;
pub					yo = {0};

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
plot_maker(const void *buffer, size_t length)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;
	int16_t		result = 0;
	/* initial position is middle of the screen */
	static int x1, x2, y1, y2;

	i = 0;
	data = (int16_t *)buffer;
	/* checks the number of samples; total size / size of 1 sample (2 byteshere) */
	number_samples= length / sizeof(int16_t);
	/* printf("%llu\n", number_samples); */
	if (length == 0)
		return;
	number_samples--;
	printf("%llu\n", number_samples); 
	int factor = 40;
	while (++i < number_samples)
	{
		yo.x1 = i * WINDOW_WIDTH / number_samples;
		yo.y1 = (WINDOW_HEIGHT / 2) - ((data[i]*factor) * WINDOW_HEIGHT/2) / 32768;
		yo.x2 =((i + 1) * WINDOW_WIDTH) / number_samples;
		yo.y2 = (WINDOW_HEIGHT / 2) - ((data[i+1]*factor) * WINDOW_HEIGHT/2) / 32768;
		/* if (result >= 0) */
		{
			SDL_SetRenderDrawColor(g_inst.renderer, 250, 0, 0, 255);
			SDL_RenderLine(g_inst.renderer, yo.x1, yo.y1, yo.x2, yo.y2);
		}
		/* printf("%d, %d, %d, %d\n", yo.x1, yo.x2, yo.y1, yo.y2); */
        /*
		 * if (result > 3000)
		 * 	printf("data: %d\n", result);
         */
	}
}

void RenderScene(SDL_Renderer* renderer, Camera2D* cam)
{
    // Clear the screen
    SDL_RenderClear(renderer);
    
    // Apply the camera transformations
    apply_camera(cam, renderer);
    
    // Render your objects here
    // Example: Render a rectangle
    SDL_FRect rect = { 100.0f, 100.0f, 50.0f, 50.0f }; // This is the world position
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    // Present the renderer
    SDL_RenderPresent(renderer);
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
    
	/* malloc c_data.buffer !! */
	AudioData c_data = 
		link_data_capture(dev_capture, dev_capture.stream, dev_capture.spec);
    
	init_wav_header(&c_data.header, c_data.spec); /* should be done when saving */
	/* AudioData a_data = load_wav("audio.wav"); */
	/* a_data.stream = init_audio_stream(&dev_output, dev_output.spec, OUTPUT); */
	{ /* global setup */
		g_inst.button.rect = 
			(SDL_FRect){.x = 200.0f, .y = 150.0f, .w = 200.0f, .h = 100.0f};
		g_inst.button.hovered = 0;
		g_inst.button.pressed = 0; 
		g_inst.button.released = 0; 
		g_inst.stream = c_data.stream;
		g_inst.out_id = dev_output.logical_id;
		g_inst.capture_id = dev_capture.logical_id;
		/* g_buffer = malloc(FIRST_ALLOC); */
		/* memset(g_buffer, 0, FIRST_ALLOC); */
	}

	Camera2D cam;
	init_camera(&cam, 0, 0, 1.0f);
	g_inst.cam = &cam;

	while (g_running)
	{
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
        /*
		 * if (g_retrieving == 0)
		 * 	vizualize_stream_data(&c_data, c_data.stream);
         */
		if (g_retrieving == 0)
			retrieve_stream_data(&c_data, c_data.stream, 1);

		apply_camera(&cam, g_inst.renderer);
		SDL_FRect rect = { 100.0f, 100.0f, 50.0f, 50.0f }; // This is the world position
		SDL_SetRenderDrawColor(g_inst.renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(g_inst.renderer, &rect);

		loop_check_button();
		Events(g_inst.e, &c_data);

		SDL_RenderPresent(g_inst.renderer);
		SDL_Delay(16);
	}
	/* SDL_free(a_data.buffer); */
	free(c_data.buffer);
	cleanup();
	return 0;
}

void
cleanup(void)
{
	/* SDL_DestroyTexture(g_inst.texture); */
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
