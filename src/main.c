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
int					g_sending = 1;
void				*g_buffer;
t_wav				g_wav_header;

typedef struct pub
{
	int x1, x2, y1, y2;
}pub;
pub					yo = {0};

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
make_realtime_plot(const void *buffer, size_t length)
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
	/* printf("%llu\n", number_samples);  */
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
	}
}

void
render_wave(Gui_audio_wave *wave, const void *buffer, int length)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;
	SDL_FPoint	*points;
	int x1, x2, y1, y2;

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
	points = malloc(sizeof(SDL_FPoint)*number_samples);
	while (i < number_samples)
	{
		x1 = i * wave->w / number_samples;
		y1 = (wave->h / 2) - ((data[i]*factor) * wave->h/2) / 32768;
		x2 =((i + 1) * wave->w) / number_samples;
		y2 = (wave->h / 2) - ((data[i+1]*factor) * wave->h/2) / 32768;
		points[i] = (SDL_FPoint){.x = x1, .y = y1};
		i++;
	}
	SDL_SetRenderTarget(g_inst.renderer, wave->text);

	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);

	SDL_SetRenderDrawColor(g_inst.renderer, 180, 90, 38, 255);
	SDL_RenderLines(g_inst.renderer, points, number_samples);
	SDL_SetRenderTarget(g_inst.renderer, NULL);
	/* SDL_RenderTexture(g_inst.renderer, wave->text, NULL, NULL); */
	SDL_RenderTexture(g_inst.renderer, wave->text, NULL, &(SDL_FRect){.x = 0, .y = 0, .w = wave->w, .h = wave->h});
	free(points);
}

/* g_buffer = malloc(FIRST_ALLOC); */
/* memset(g_buffer, 0, FIRST_ALLOC); */

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

	init_wav_header(&c_data.header, c_data.spec); /* should be done when saving */

	g_inst.button.rect = (SDL_FRect){.x = 200.0f, .y = 400.0f, .w = 200.0f, .h = 100.0f};
	Camera2D cam;
	init_camera(&cam, 0, 0, 1.0f);
	g_inst.cam = &cam;

	SDL_RendererInfo r_info;
	SDL_GetRendererInfo(g_inst.renderer, &r_info);
	printf("renderer's name = %s\n", r_info.name);
	Gui_audio_wave wave = { .text = NULL, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT/4};
	wave.text = SDL_CreateTexture(g_inst.renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);
	SDL_SetRenderTarget(g_inst.renderer, wave.text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);

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
		SDL_RenderTexture(g_inst.renderer, wave.text, NULL, &(SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h});

		SDL_RenderPresent(g_inst.renderer);
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
