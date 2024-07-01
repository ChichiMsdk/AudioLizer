#include "app.h"

#ifdef WIN_32
	#include			<windows.h>
	LARGE_INTEGER		wfreq;
	LARGE_INTEGER		wstart;
	LARGE_INTEGER		wend;
	float				welapsed;
#endif

void
set_new_frame(SDL_Color c)
{
	SDL_SetRenderTarget(g_inst.r, NULL);
	SDL_SetRenderDrawColor(g_inst.r, c.r, c.g, c.b, c.a);
	SDL_RenderClear(g_inst.r);
}

void
make_realtime_plot(const void *buffer, size_t length)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;
	int16_t		result = 0;
	/* initial position is middle of the screen */
	static int x1, x2, y1, y2;
	int w_width = g_win_w;
	int w_height = g_win_h;

	i = 0;
	data = (int16_t *)buffer;
	/* the number of samples; total size/(size of 1 sample) (2 byteshere) */
	number_samples = length / sizeof(int16_t);
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

Audio_wave
resize_texture(SDL_Texture *texture)
{
	SDL_DestroyTexture(texture);
	Audio_wave wave = {.text = NULL, .w = g_win_w, .h = g_win_h, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, g_win_w, g_win_h/2);
	wave.rect = (SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h};

	SDL_SetRenderTarget(g_inst.r, wave.text);
	YU_SetRenderDrawColor(g_inst.r, YU_RED);
	SDL_RenderClear(g_inst.r);
	return wave;
}

Audio_wave
init_audio_wave(void)
{
	Audio_wave wave = {.text = NULL, .w = g_win_w, .h = g_win_h, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	wave.rect = (SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h};
	SDL_SetRenderTarget(g_inst.r, wave.text);
	YU_SetRenderDrawColor(g_inst.r, YU_GRAY);
	SDL_RenderClear(g_inst.r);
	return wave;
}

SDL_Texture*
init_svg(char const *arr, int w, int h)
{
	SDL_IOStream *fsvg = SDL_IOFromConstMem(arr, strlen(arr));
	if (!fsvg)
		logExit("Load svg from mem failed");
	SDL_Surface *ssvg = IMG_LoadSizedSVG_IO(fsvg, w, h);
	if (!ssvg)
		logExit("Load svg failed");
	SDL_Texture *text = SDL_CreateTextureFromSurface(g_inst.r, ssvg);
	if (!text)
		logExit("Load text from surface failed");
	SDL_DestroySurface(ssvg);
	if (SDL_CloseIO(fsvg))
		logExit("Could not close fsvg");
	return text;
}


void
print_playlist(void)
{
	if (g_playlist.size == 0)
	{
		printf("No songs!\n");
		return ;
	}
	int i = 0;
	assert(g_playlist.size <= MAX_BUFFER_SIZE);
	assert(g_playlist.size >= 0);
	printf("current: %d\n", g_playlist.current);
	while (i < g_playlist.size)
	{
		printf("%d/%d\n", i + 1, g_playlist.size);
		printf("%s\n", g_playlist.music[i].name);
		i++;
	}
}

void
draw_dynamic_text(font *f)
{
	if (g_playlist.size == 0)
	{
		char *str = "No songs";
		size_t len = (strlen(str)/2)*f->data.glyphs[1].w;
		font_write(f, g_inst.r, (SDL_Point){(g_win_w/2)-len, 100}, str);
		return ;
	}
	int visible_count = (g_win_h - 200) / f->data.glyphs[1].h;
	int selected_index = g_playlist.current;
	int start_index = selected_index - (visible_count / 2);

	if (start_index < 0)
		start_index = 0;
	if (start_index > g_playlist.size - visible_count)
		start_index = g_playlist.size - visible_count;

	if (start_index < 0)
		start_index = 0;
	int i = 0;
	int j = start_index;
	assert(g_playlist.size <= MAX_BUFFER_SIZE);
	assert(g_playlist.size >= 0);
	while (i < visible_count && j < g_playlist.size)
	{
		if (j == g_playlist.current)
			f->color = (SDL_Color){158, 149, 199};
		else
			f->color = (SDL_Color){255, 255, 255, 255};
		font_write(f, g_inst.r, (SDL_Point){60, (f->data.glyphs[1].h*i) + 50}, g_playlist.music[j].name);
		j++;
		i++;
	}
}

void
draw_text_music(SDL_FRect p, SDL_Color c, char const *msg, SDL_Texture *tex)
{
	SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
	SDL_RenderTexture(g_inst.r, tex, NULL, &p);
}

void
draw_text_texture(SDL_Point p, SDL_Color c, char const *msg, SDL_Texture *tex)
{
	int w = 0, h = 0;
	TTF_SizeText(g_inst.ttf, msg, &w, &h);
	SDL_FRect rect = {.x = p.x, .y = p.y, .w = w, .h = h};
	SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
	SDL_RenderTexture(g_inst.r, tex, NULL, &rect);
}

void
draw_playlist(font *f)
{
	if (g_playlist.size == 0)
	{
		draw_text_music(g_inst.nosongs.r, YU_WHITE, "No songs", g_inst.nosongs.texture);
		return ;
	}
	int visible_count = (g_win_h - 200) / f->data.glyphs[1].h;
	int selected_index = g_playlist.current;
	int start_index = selected_index - (visible_count / 2);

	if (start_index < 0) start_index = 0;
	if (start_index > g_playlist.size - visible_count)
		start_index = g_playlist.size - visible_count;

	if (start_index < 0) start_index = 0;
	int i = 0, j = start_index;
	assert(g_playlist.size <= MAX_BUFFER_SIZE);
	assert(g_playlist.size >= 0);
	SDL_Color c = {0};
	SDL_Point p = {.x = 60};
	while (i < visible_count && j < g_playlist.size)
	{
		if (j == g_playlist.current)
			c = (SDL_Color){158, 149, 199, 255};
		else
			c = (SDL_Color){255, 255, 255, 255};
		p.y = (f->data.glyphs[1].h*i) + 50;
		g_playlist.music[j].rect.y = p.y;
		g_playlist.music[j].rect.x = p.x;
		draw_text_music(g_playlist.music[j].rect, c, g_playlist.music[j].name, g_playlist.music[j].texture);
		j++;
		i++;
	}
}

void
draw_fps(font *f)
{
	/* note : change x/y to be relative rather than absolute */
	SDL_Point p = {.x = g_win_w - 200, .y = 32};
	char fpsText[20];
	sprintf(fpsText, "%llu", g_fps);
	font_write(f, g_inst.r, p, fpsText);
}

void
count_fps(font *f)
{
	g_frame_count++;
	g_end = SDL_GetTicksNS();
	if (((float)(g_end - g_start) / (1000 * 1000 * 1000)) >= 1 )
	{
		g_fps = g_frame_count;
		g_frame_count = 0;
		g_start = g_end;
	}
	draw_fps(f);
}

void
draw_wave_raw(Uint8 *dst)
{
	SDL_LockMutex(g_inst.w_form.mutex);
	if (g_inst.w_form.open == false)
		goto end;
	YU_MixAudio(dst, (Uint8*)g_inst.w_form.buffer, SDL_AUDIO_F32,
			g_inst.w_form.buflen, g_test, &g_inst.w_form.wave);

	g_inst.w_form.open = false;
end:
	SDL_FRect view = {.x = 0, .y = g_win_h - g_inst.w_form.wave.h, .w = g_inst.w_form.wave.w, .h = g_inst.w_form.wave.h};	
	SDL_SetRenderTarget(g_inst.r, NULL);
	SDL_RenderTexture(g_inst.r, g_inst.w_form.wave.text, NULL, &view);
	SDL_UnlockMutex(g_inst.w_form.mutex);
}

void
draw_button2(Button *button)
{
	uint8_t a, b, c;
	if (button->pressed)
	{
		/* SDL_SetTextureColorMod(button->text[1], 166, 166, 255); */
		SDL_FRect r = button->rect;
		r.y = g_win_h - 150; 
		int cx = r.x + r.w /2;
		int cy = r.y + r.h /2;
		r.h *= 0.9f;
		r.w *= 0.9f;
		r.x = cx - r.w / 2;
		r.y = cy - r.h / 2;
		SDL_SetTextureColorMod(button->text[1], 66, 66, 155);
		SDL_RenderTexture(g_inst.r, button->text[1], NULL, &r);
		SDL_SetTextureColorMod(button->text[1], 255, 0, 0);
	}
	else if (button->hovered)
	{
		SDL_FRect r = button->rect;
		r.y = g_win_h - 150; 
		int cx = r.x + r.w /2;
		int cy = r.y + r.h /2;
		r.h *= 1.1f;
		r.w *= 1.1f;
		r.x = cx - r.w / 2;
		r.y = cy - r.h / 2;
		SDL_SetTextureColorMod(button->text[0], 240, 240, 240);
		SDL_RenderTexture(g_inst.r, button->text[0], NULL, &button->rect);
		SDL_SetRenderDrawColor(g_inst.r, 41, 41, 41, 80);
		SDL_RenderFillRect(g_inst.r, &r);
	}
	else
	{
		button->rect.y = g_win_h - 150;
		SDL_SetTextureColorMod(button->text[0], 240, 240, 240);
		SDL_RenderTexture(g_inst.r, button->text[0], NULL, &button->rect);
	}
}

void
draw_buttons(Button *buttons)
{
	int i = 0;
	while (i < buttons->count)
	{
		draw_button2(&buttons[i]);
		i++;
	}
}

	/* note: add viewport to the render_wave so we can slide it! */
	/*
	 * void
	 * render_wave(Audio_wave *wave, const void *buf, int len, SDL_AudioSpec spec)
	 * {
	 * 	int			 x1, x2, y1, y2;
	 * 	int 		factor = 20;
	 * 	size_t		i = 0;
	 * 	Uint8		*dst;
	 * 	Uint8		*buffer = buf;
	 * 	int			length = len;
	 * 
	 * 	#<{(| size_t		number_samples = 10000; |)}>#
	 * 	size_t		number_samples = get_samples(spec);
	 * 	SDL_FPoint	*points;
	 * 	SDL_FRect	view = {.x = 0, .y = 0, .w = wave->w, .h = wave->h};
	 * 	printf("%d %d\n", wave->w, wave->h);
	 * 
	 * 	if (length == 0)
	 * 		return;
	 * 	dst = malloc(sizeof(Uint8) * len);
	 * 	points = malloc(sizeof(SDL_FPoint) * len);
	 * 	memset(points, 0, len);
	 * 
	 * 	while (i < number_samples)
	 * 	{
	 * 		x1 = i * wave->w / number_samples;
	 * 		y1 = (wave->h / 2) - ((dst[i]*factor) * wave->h/2) / 32768;
	 * 		points[i] = (SDL_FPoint){.x = x1, .y = y1};
	 * 		i++;
	 * 	}
	 * 	printf("i: %llu\n", i);
	 * 	SDL_SetRenderTarget(g_inst.r, wave->text);
	 * 	SDL_SetRenderDrawColor(g_inst.r, 50, 50, 50, 255);
	 * 	SDL_RenderClear(g_inst.r);
	 * 	SDL_SetRenderDrawColor(g_inst.r, 180, 90, 38, 255);
	 * 	SDL_RenderLines(g_inst.r, points, i);
	 * 
	 * 	SDL_SetRenderTarget(g_inst.r, NULL);
	 * 	SDL_RenderTexture(g_inst.r, wave->text, NULL, &view);
	 * 	free(points);
	 * 	free(dst);
	 * }
	 */
	/*
	 * void
	 * draw_button(Button button)
	 * {
	 * 	#<{(| just for convenience |)}>#
	 * 		Uint8 r_p = button.color_pressed.r; 	
	 * 		Uint8 g_p = button.color_pressed.g; 	
	 * 		Uint8 b_p = button.color_pressed.b; 	
	 * 		Uint8 a_p = button.color_pressed.a; 	
	 * 		Uint8 r = button.color.r; 	
	 * 		Uint8 g = button.color.g; 	
	 * 		Uint8 b = button.color.b; 	
	 * 		Uint8 a = button.color.a; 	
	 * 
	 * 	if (button.pressed)
	 * 	{
	 * 		SDL_SetRenderDrawColor(g_inst.r, r_p, g_p, b_p, a_p);
	 * 		#<{(| SDL_SetRenderDrawColor(g_inst.renderer, 100, 200, 50, 255); |)}>#
	 * 	}
	 * 	else if (button.hovered)
	 * 	{
	 * 		SDL_SetRenderDrawColor(g_inst.r, r, g, b, 80);
	 * 	}
	 * 	else
	 * 		SDL_SetRenderDrawColor(g_inst.r, r, g, b, a);
	 * 
	 * 	SDL_SetRenderTarget(g_inst.r, g_inst.texture);
	 * 	SDL_RenderFillRect(g_inst.r, &button.rect);
	 * 	SDL_SetRenderTarget(g_inst.r, NULL);
	 * }
	 */
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
