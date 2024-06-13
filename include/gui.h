#ifndef GUI_H
#define GUI_H

#include "audio.h"
#include "vector.h"

#include <SDL3/SDL.h>

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
	size_t				current;
}Audio_wave;

typedef struct Button
{
	SDL_FRect			rect;
	SDL_Color			color;
	SDL_Color			color_pressed;
	bool				pressed, released, hovered;
	void				*(*fn)(void *);
	int					count;
}Button;

enum
{
	B_UP = 0,
	B_DOWN = 1
};

void					button_check_pressed(Mouse_state mouse, Button *button);
void					button_check_released(Mouse_state mouse, Button *button);
void					button_check_hover(Mouse_state mouse, Button *button);

void					make_realtime_plot(const void *buffer, size_t length);
void					load_to_stream(AudioData *sfx);

void					render_wave(Audio_wave *wave, const void *buffer, int length);
#endif
