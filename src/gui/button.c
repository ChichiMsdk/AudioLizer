#include "audio.h"

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
loop_check_button(void)
{
	if (g_inst.button.hovered)
	{
		SDL_SetRenderDrawColor(g_inst.renderer, 100, 50, 50, 100);
		if (g_inst.button.pressed)
			SDL_SetRenderDrawColor(g_inst.renderer, 50, 170, 50, 100);
	}
	else
		SDL_SetRenderDrawColor(g_inst.renderer, 170, 50, 50, 255);

	SDL_SetRenderTarget(g_inst.renderer, g_inst.texture);
	SDL_RenderFillRect(g_inst.renderer, &g_inst.button.rect);
	SDL_SetRenderTarget(g_inst.renderer, NULL);
}

void
button_check(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	/* checks if inside button */
	if ((m_pos.x < button->rect.w + button->rect.x
			&& m_pos.x >= button->rect.x)
			&& (m_pos.y < button->rect.h + button->rect.y
			&& m_pos.y >= button->rect.y)) 
	{
		button->hovered = true;
		/* create array of cursor beforehand */
		g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
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
		g_inst.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
		SDL_SetCursor(g_inst.cursor);
	}
}
