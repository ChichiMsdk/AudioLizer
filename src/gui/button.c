#include "audio.h"

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
