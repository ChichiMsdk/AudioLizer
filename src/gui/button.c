#include "app.h"

int					g_playing;

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

/* data struct to loop through all the buttons */
void
loop_gui(Button **buttons)
{

}

void
draw_button(Button button)
{
	Uint8 r_p = button.color_pressed.r; 	
	Uint8 g_p = button.color_pressed.g; 	
	Uint8 b_p = button.color_pressed.b; 	
	Uint8 a_p = button.color_pressed.a; 	
	Uint8 r = button.color.r; 	
	Uint8 g = button.color.g; 	
	Uint8 b = button.color.b; 	
	Uint8 a = button.color.a; 	

	if (button.pressed)
	{
		SDL_SetRenderDrawColor(g_inst.renderer, r_p, g_p, b_p, a_p);
		/* SDL_SetRenderDrawColor(g_inst.renderer, 100, 200, 50, 255); */
	}
	else if (button.hovered)
	{
		SDL_SetRenderDrawColor(g_inst.renderer, r, g, b, 80);
	}
	else
		SDL_SetRenderDrawColor(g_inst.renderer, r, g, b, a);

	SDL_SetRenderTarget(g_inst.renderer, g_inst.texture);
	SDL_RenderFillRect(g_inst.renderer, &button.rect);
	SDL_SetRenderTarget(g_inst.renderer, NULL);
}

void
button_check_hover(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	int i = 0;
	/* checks if inside button */
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			button[i].hovered = true;
			/* create array of cursor beforehand */
			g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
			SDL_SetCursor(g_inst.cursor);
			break;
		}
		else
		{
			button[i].hovered = false;
			/* button[i].pressed = false; */
			SDL_DestroyCursor(g_inst.cursor);
			g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
			SDL_SetCursor(g_inst.cursor);
		}
		i++;
	}
}

/* note: make a function pointer callback for the button when pressed */
void
button_check_pressed(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	int i = 0;
	/* checks if inside button */
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			/* create array of cursor beforehand */
			if (m_button == SDL_BUTTON_LEFT)
			{
				button[i].pressed = true;
			}
		}
		else
		{
			button[i].pressed = false;
		}
		i++;
	}
}

void*
play_pause(void *id_void)
{
	SDL_AudioDeviceID id = *(SDL_AudioDeviceID*) id_void;
	/* g_playing = !g_playing; */
	if (SDL_AudioDevicePaused(id))
	{
		printf("Audio Playing\n");
		SDL_ResumeAudioDevice(id);
	}
	else
	{
		printf("Audio Paused\n");
		SDL_PauseAudioDevice(id);
	}
	return NULL;
}

void
button_check_released(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);
	/* checks if inside button */
	int i = 0;
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			if (button[i].pressed)
			{
				{
				/* create array of cursor beforehand */
					button[i].released = true;
					if (button[i].fn)
						button[i].fn(&g_inst.out_id);
				}
			}
		}
		button[i].pressed = false;
		i++;
	}
}
