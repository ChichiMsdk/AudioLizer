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

void
button_check(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = mouse.pos;
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	/* checks if inside button */
	if (((int)m_pos.x < button->rect.w + button->rect.x
			&& (int)m_pos.x >= button->rect.x)
			&& ((int)m_pos.y < button->rect.h + button->rect.y
			&& (int)m_pos.y >= button->rect.y)) 
	{
		button->hovered = true;
		/* create array of cursor beforehand */
		g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		SDL_SetCursor(g_inst.cursor);
		if (m_button == SDL_BUTTON_LEFT)
		{
			button->pressed = true;
		}
		else
			button->pressed = false;
	}
	else
	{
		button->hovered = false;
		button->pressed = false;
		SDL_DestroyCursor(g_inst.cursor);
		g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		SDL_SetCursor(g_inst.cursor);
	}
}

int
/* WinMain(int ac, char **av) */
main(int ac, char **av)
{
	/* gets potential value for devices */
	if (ac >= 2)
	{ g_inst.capture_name = av[1]; if (ac >=3) g_inst.output_name = av[2]; }
	init_sdl();
	init_audio();
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
        /*
		 * if (g_retrieving == 0)
		 * 	retrieve_stream_data();
         */

		if (g_inst.button.hovered)
		{
			SDL_SetRenderDrawColor(g_inst.renderer, 100, 50, 50, 100);
			if (g_inst.button.pressed)
				SDL_SetRenderDrawColor(g_inst.renderer, 50, 170, 50, 100);
		}
		else
			SDL_SetRenderDrawColor(g_inst.renderer, 170, 50, 50, 255);

		SDL_RenderFillRect(g_inst.renderer, &g_inst.button.rect);

		Events(g_inst.e);

		SDL_RenderPresent(g_inst.renderer);
	}
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
	free(g_buffer);
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
