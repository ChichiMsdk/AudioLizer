#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "vector.h"

typedef struct Mouse_state
{
	Vec2f				pos;
	uint32_t			flags;
}Mouse_state;

typedef struct Gui_audio_wave
{
	SDL_Texture			*text;
	int					w;
	int					h;
}Gui_audio_wave;

typedef struct Button
{
	SDL_FRect			rect;
	bool				pressed, released, hovered;
}Button;

void					button_check(Mouse_state mouse, Button *button);

#endif
