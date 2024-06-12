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
			SDL_SetRenderDrawColor(g_inst.renderer, 250, 0, 0, 255);
			SDL_RenderLine(g_inst.renderer, x1, y1, x2, y2);
		}
		/* printf("%d, %d, %d, %d\n", x1, x2, y1, y2); */
	}
}

/* note: add viewport to the render_wave so we can slide it! */
void
render_wave(Audio_wave *wave, const void *buffer, int length)
{
	int			 x1, x2, y1, y2;
	int 		factor = 40;
	size_t		i = 0;
	int16_t		*data = (int16_t *)buffer;
	size_t		number_samples = (length / sizeof(int16_t)) - 1;
	SDL_FPoint	*points = malloc(sizeof(SDL_FPoint)*number_samples);
	SDL_FRect	view = {.x = 0, .y = 0, .w = wave->w, .h = wave->h};

	if (length == 0)
		return;
	while (i < number_samples)
	{
		x1 = i * wave->w / number_samples;
		y1 = (wave->h / 2) - ((data[i]*factor) * wave->h/2) / 32768;
		x2 =((i + 1) * wave->w) / number_samples;
		y2 = (wave->h / 2) - ((data[i+1]*factor) * wave->h/2) / 32768;
		points[i] = (SDL_FPoint){.x = x1, .y = y1};
		i++;
	}
	SDL_SetRenderTarget(g_inst.renderer, wave->text);
	SDL_SetRenderDrawColor(g_inst.renderer, 50, 50, 50, 255);
	SDL_RenderClear(g_inst.renderer);
	SDL_SetRenderDrawColor(g_inst.renderer, 180, 90, 38, 255);
	SDL_RenderLines(g_inst.renderer, points, number_samples);

	SDL_SetRenderTarget(g_inst.renderer, NULL);
	SDL_RenderTexture(g_inst.renderer, wave->text, NULL, &view);
	free(points);
}

void
load_to_stream(AudioData *sfx)
{
	SDL_PutAudioStreamData(sfx->stream, sfx->buffer, sfx->length);
	return;
	size_t			total = sfx->length;
	size_t			buflen = 0;
	size_t			sample = 4096;
	SDL_AudioStream	*stream = g_inst.stream;
	char			buf[4096];
	static size_t	i;
	size_t			j = 0;

	total--;
	while (i + j < total && j < sample)
	{
		j++;
	}
	if (j == 0)
	{
		g_playing = 1;
		printf("size read: %llu KB\n", i/1000);
		return ;
	}
	printf("%llu\n", j);
	memcpy(buf, sfx->buffer+i, j);
	i += j;
	SDL_PutAudioStreamData(stream, buf, j);
}
