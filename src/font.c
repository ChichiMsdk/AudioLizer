/* 
 * great website, very helpful
 * https://ctor.tv/blog/spooky-glyph-atlas/
 */
#include "font.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void
render_font()
{
}

#ifdef WIN_32
#include <windows.h>
void print_timer(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq);
LARGE_INTEGER wavg = {0};
LARGE_INTEGER wcount = {0};
#endif

static const glyph*
font_putchar(font *self, SDL_Renderer *renderer, SDL_Point dest, char text)
{
	if(text == '\0') 
		return NULL; 
	/* Lookup the glyph from the character */
	const glyph *g = &self->data.glyphs[(size_t)text];
	SDL_FRect d = { .x = dest.x, .y = dest.y, .w = g->w, .h = self->data.height };
	SDL_SetTextureColorMod(g->texture, self->color.r, self->color.g, self->color.b);
	SDL_RenderTexture(renderer, g->texture, NULL, &d);
	return g;
}

/*
 * This is for dynamic text that will frequently change!
 * If text will remain static, just use TTF_RenderUTF8_Blended_Wrapped
 */
void
font_write(font *self, SDL_Renderer *renderer, SDL_Point point, const char *text) 
{
	const char *t = text;
	/* QueryPerformanceCounter(&wstart); */
	while(*t != '\0')
	{
		const glyph *g = font_putchar(self, renderer, point, *t);
		point.x += g->w;
		t++;
	} 
	/* QueryPerformanceCounter(&wend); */
	/* print_timer(wstart, wend, wfreq); */
}

static void 
font_glyph_create_texture(TTF_Font *ttf, SDL_Renderer *renderer, glyph *g) 
{
	static const SDL_Color foreground = { .r = 255, .g = 255, .b = 255, .a = 255 };
	if(!g->c || *(g->c) == '\0') 
	{ 
		return; 
	}
	SDL_Surface * surface = TTF_RenderUTF8_Blended_Wrapped(ttf, g->c, foreground, 0);
	g->texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);
	surface = NULL;
}

static void
font_build_atlas(font *self, TTF_Font *ttf, SDL_Renderer *renderer)
{
	font_data *data = &(self->data);
	for(size_t i = 0; i < data->glyphs_capacity; i++)
	{
		assert(data->text_buf_len + 1 < data->text_buf_capacity);
		/* if(data->text_buf_len + 1 >= data->text_buf_capacity) { abort(); } */

		glyph *g = data->glyphs + i;
		g->c = data->text_next;
		g->c[0] = (char)i; /* laziness */
		g->c[1] = '\0';

		data->text_next += sizeof * g->c + sizeof '\0';
		data->text_buf_len++;

		font_glyph_create_texture(ttf, renderer, g);

		TTF_SizeUTF8(data->font, g->c, &(g->w), &(g->h));
		data->glyphs_len++;
	}
}

/*
 * Must destroy the texture once finished with it
 */
SDL_Texture *
create_static_text(TTF_Font *ttf, SDL_Renderer *r, const char *text)
{
	SDL_Color color = { .r = 255, .g = 255, .b = 255, .a = 255};
	SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(ttf, text, color, 0);
	if (!s)
		return fprintf(stderr, "Surface text could not be created\n"), NULL;
	SDL_Texture *texture = SDL_CreateTextureFromSurface(r, s);
	SDL_DestroySurface(s);
	return texture;
}

void
init_font(font *self, SDL_Renderer *renderer, TTF_Font *ttf)
{
	font_data *data = &(self->data);
	self->color = (SDL_Color){255, 255, 255};

	data->font = ttf;
	data->height = TTF_FontHeight(ttf);

	data->glyphs_len = 0;
	data->glyphs_capacity = 256;
	data->text_buf_len = 0;
	data->text_buf_capacity = 2048;

	data->glyphs = calloc(data->glyphs_capacity, sizeof * data->glyphs);
	if(!data->glyphs) { abort(); }

	data->text_buf = calloc(data->text_buf_capacity, sizeof * data->text_buf);
	if(!data->text_buf) { abort(); }
	data->text_next = data->text_buf;

	font_build_atlas(self, ttf, renderer);
}
