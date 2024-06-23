#include "app.h"

int					g_playing;

void
make_realtime_plot(const void *buffer, size_t length)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;
	int16_t		result = 0;
	/* initial position is middle of the screen */
	static int x1, x2, y1, y2;
	int w_width = WINDOW_WIDTH;
	int w_height = WINDOW_HEIGHT;

	i = 0;
	data = (int16_t *)buffer;
	/* the number of samples; total size/(size of 1 sample) (2 byteshere) */
	number_samples= length / sizeof(int16_t);
	/* printf("%llu\n", number_samples); */
	if (length == 0)
		return;
	number_samples--;
	/* printf("%llu\n", number_samples);  */
	int factor = 40;
	while (++i < number_samples)
	{
		x1 = i * w_width / number_samples;
		y1 = (w_height / 2) - ((data[i]*factor) * w_height/2) / 32768;
		x2 =((i + 1) * w_width) / number_samples;
		y2 = (w_height / 2) - ((data[i+1]*factor) * w_height/2) / 32768;
		/* if (result >= 0) */
		{
			SDL_SetRenderDrawColor(g_inst.r, 250, 0, 0, 255);
			SDL_RenderLine(g_inst.r, x1, y1, x2, y2);
		}
		/* printf("%d, %d, %d, %d\n", x1, x2, y1, y2); */
	}
}

/* note: add viewport to the render_wave so we can slide it! */
void
render_wave(Audio_wave *wave, const void *buffer, int length)
{
	int			 x1, x2, y1, y2;
	int 		factor = 20;
	size_t		i = 0;
	int16_t		*data = (int16_t *)buffer;
	/* size_t		number_samples = 10000; */
	size_t		number_samples = (length / sizeof(int16_t)) - 1;
	SDL_FPoint	*points;
	SDL_FRect	view = {.x = 0, .y = 0, .w = wave->w, .h = wave->h};

	if (length == 0)
		return;
	points = malloc(sizeof(SDL_FPoint) * number_samples);

	while (i < number_samples)
	{
		x1 = i * wave->w / number_samples;
		y1 = (wave->h / 2) - ((data[i]*factor) * wave->h/2) / 32768;
		points[i] = (SDL_FPoint){.x = x1, .y = y1};
		i++;
	}
	SDL_SetRenderTarget(g_inst.r, wave->text);
	SDL_SetRenderDrawColor(g_inst.r, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.r);
	SDL_SetRenderDrawColor(g_inst.r, 180, 90, 38, 255);
	SDL_RenderLines(g_inst.r, points, number_samples);

	SDL_SetRenderTarget(g_inst.r, NULL);
	SDL_RenderTexture(g_inst.r, wave->text, NULL, &view);
	free(points);
}

void
draw_button2(Button button)
{
	uint8_t a, b, c;
	if (button.pressed)
	{
		/* SDL_SetTextureColorMod(button.text[1], 166, 166, 255); */
		SDL_FRect r = button.rect;
		int cx = r.x + r.w /2;
		int cy = r.y + r.h /2;
		r.h *= 0.9f;
		r.w *= 0.9f;
		r.x = cx - r.w / 2;
		r.y = cy - r.h / 2;
		SDL_SetTextureColorMod(button.text[1], 66, 66, 155);
		SDL_RenderTexture(g_inst.r, button.text[1], NULL, &r);
		SDL_SetTextureColorMod(button.text[1], 255, 0, 0);
		SDL_GetTicks();
	}
	else if (button.hovered)
	{
		SDL_FRect r = button.rect;
		int cx = r.x + r.w /2;
		int cy = r.y + r.h /2;
		r.h *= 1.1f;
		r.w *= 1.1f;
		r.x = cx - r.w / 2;
		r.y = cy - r.h / 2;
		SDL_SetTextureColorMod(button.text[0], 240, 240, 240);
		SDL_RenderTexture(g_inst.r, button.text[0], NULL, &button.rect);
		SDL_SetRenderDrawColor(g_inst.r, 41, 41, 41, 80);
		SDL_RenderFillRect(g_inst.r, &r);
	}
	else
	{
		SDL_SetTextureColorMod(button.text[0], 240, 240, 240);
		SDL_RenderTexture(g_inst.r, button.text[0], NULL, &button.rect);
	}
}

void
draw_button(Button button)
{
	/* just for convenience */
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
		SDL_SetRenderDrawColor(g_inst.r, r_p, g_p, b_p, a_p);
		/* SDL_SetRenderDrawColor(g_inst.renderer, 100, 200, 50, 255); */
	}
	else if (button.hovered)
	{
		SDL_SetRenderDrawColor(g_inst.r, r, g, b, 80);
	}
	else
		SDL_SetRenderDrawColor(g_inst.r, r, g, b, a);

	SDL_SetRenderTarget(g_inst.r, g_inst.texture);
	SDL_RenderFillRect(g_inst.r, &button.rect);
	SDL_SetRenderTarget(g_inst.r, NULL);
}

void
draw_buttons(Button *buttons)
{
	int i = 0;
	/* note: count of buttons*/
	while (i < buttons->count)
	{
		/* draw_button(buttons[i]); */
		draw_button2(buttons[i]);
		i++;
	}
}

/*
 * void
 * load_to_stream(AudioData *sfx)
 * {
 * 	SDL_PutAudioStreamData(sfx->stream, sfx->buffer, sfx->length);
 * 	return;
 * 	size_t			total = sfx->length;
 * 	size_t			buflen = 0;
 * 	size_t			sample = 4096;
 * 	SDL_AudioStream	*stream = g_inst.stream;
 * 	char			buf[4096];
 * 	static size_t	i;
 * 	size_t			j = 0;
 * 
 * 	total--;
 * 	while (i + j < total && j < sample)
 * 	{
 * 		j++;
 * 	}
 * 	if (j == 0)
 * 	{
 * 		g_playing = 1;
 * 		printf("size read: %llu KB\n", i/1000);
 * 		return ;
 * 	}
 * 	printf("%llu\n", j);
 * 	memcpy(buf, sfx->buffer+i, j);
 * 	i += j;
 * 	SDL_PutAudioStreamData(stream, buf, j);
 * }
 */
