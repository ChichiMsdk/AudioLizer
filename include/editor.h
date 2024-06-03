#pragma once

#define WIN32_LEAN_AND_MEAN 
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "vector.h"

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0

typedef struct sfx
{
	Mix_Chunk	*pressed;
	Mix_Chunk 	*released;
	int			index;
	bool		is_pressed;
}sfx;

// Check padding
typedef struct YUinstance
{
	SDL_Window			*window;
	SDL_Renderer		*renderer;
	SDL_Rect			rect;
	SDL_Event			e;
	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	cDevID;
	SDL_AudioDeviceID	oDevID;
	FILE				*audio_file;
}YUinstance;

extern YUinstance	inst;
extern int			WINDOW_WIDTH;
extern int			WINDOW_HEIGHT;
extern int			running;

// editor.c
void			Events(SDL_Event e);
void			save_file(FILE *file);

// error.c
				// window renderer surface font
void			logger(void *w, void *r, void *s, void *f, const char *msg);
