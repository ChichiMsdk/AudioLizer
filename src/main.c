#include "editor.h"
#include <stdlib.h>

YUinstance			g_inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					g_retrieving = 1;
int					g_running = 1;
int					g_saving = 1;
void				*g_buffer;
t_wav				g_wav_header;

void 
init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{ fprintf(stderr, "%s\n", SDL_GetError()); exit(1); }

	g_inst.window = SDL_CreateWindow("Key capture", WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (g_inst.window == NULL)
	{ fprintf(stderr, "%s\n", SDL_GetError()); SDL_Quit(); exit(1); }

	/* sets the global var for the capture and output logical dev */
	SDL_AudioSpec a_capture_spec = set_capture_device(g_inst.capture_name);
	SDL_AudioSpec a_output_spec = set_output_device(g_inst.output_name);

	/* same here stream is set in global g_inst.stream */
	stream_capture_init(a_capture_spec, g_inst.capture_id);

	/* should probably do this just before saving the file instead */
	wav_header_init(a_capture_spec);

	g_inst.renderer = SDL_CreateRenderer(g_inst.window,NULL);
	if (g_inst.renderer == NULL)
		logExit("renderer failed to be created");
}



void
save_file(FILE *file)
{
	static size_t bytes_written;

	/* adapt file length to new buffer length */
	g_wav_header.flength = g_wav_header.dlength + 44;
	bytes_written = fwrite(&g_wav_header, sizeof(t_wav), 1, file);
	if (bytes_written < 0)
	{ perror("fwrite line 138:"); exit(1); }

	bytes_written += fwrite(g_buffer, 1, g_wav_header.dlength, file);
	if (bytes_written < 0)
	{ perror("fwrite line 138:"); exit(1); }
	/* printf("bytes_written %fKB\n", (double) bytes_written/1000); */
}

int
/* WinMain() */
main(int ac, char **av)
{
	/* is set for the capture device sample in set_capture_device */
	g_inst.sample_size = 0;
	g_inst.current_buff_size = FIRST_ALLOC;
	g_inst.capture_name = NULL;
	g_inst.output_name = NULL;

	if (ac >= 2)
	{
		g_inst.capture_name = av[1];
		if (ac >=3)
			g_inst.output_name = av[2];
	}

	init();
	g_buffer = malloc(FIRST_ALLOC); assert(g_buffer);
	g_inst.audio_file = fopen("audio.wav", "wb");
	if (!g_inst.audio_file) { perror("Error fopen line 151: "); exit(1); }

	while (g_running)
	{
		/* wtf ? is this supposed to work like this ?..*/
		if (g_retrieving == 0)
			retrieve_stream_data();
		SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(g_inst.renderer);
		Events(g_inst.e);
		SDL_RenderPresent(g_inst.renderer);
	}
	cleanup();

	return 0;
}

void
cleanup(void)
{
	SDL_DestroyAudioStream(g_inst.stream);
	SDL_DestroyRenderer(g_inst.renderer);
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
	fclose(g_inst.audio_file);
	free(g_buffer);
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
