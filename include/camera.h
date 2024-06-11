#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "vector.h"

typedef struct Camera2D
{
	float 		x;
	float		y;
	float		scale;
	SDL_FRect	view;
}Camera2D;

void	init_camera(Camera2D *cam, float x, float y, float scale);
void	move_camera(Camera2D *cam, float dx, float dy);
void	set_scale_camera(Camera2D* cam, float scale);
void	apply_camera(Camera2D *cam, SDL_Renderer *renderer);
Vec2f	screen_to_world(Camera2D* cam, Vec2f screen);
void	zoom_camera(Camera2D *cam, float factor, int mX, int mY);

#endif
