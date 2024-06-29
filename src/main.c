#include			"app.h"
#include			"font.h"

#ifdef				WIN_32
	#include			<windows.h>
	#include			<io.h>
	LARGE_INTEGER		wfreq;
	LARGE_INTEGER		wstart;
	LARGE_INTEGER		wend;
	float				welapsed;
#endif


/* mandatory to launch app without console */
#define				SDL_MAIN_HANDLED
#include			<SDL3/SDL_main.h>

#define YU_GRAY (SDL_Color){ .r = 28, .g = 28, .b = 28, .a = 255}
#define YU_WHITE (SDL_Color){255, 255, 255, 255}

	YUinstance			g_inst = {0};
	int					g_win_w = 2400;
	int					g_win_h = 1600;
	int					g_retrieving = 1;
	int					g_vizualizing = 1;
	int					g_running = 1;
	int					g_saving = 1;
	int					g_sending = 1;
	int					g_playing = 1;
	int					buff_end = 0;
	float				g_volume = 1;
	void				*g_buffer = NULL;
	t_wav				g_wav_header = {0};
	Playlist			g_playlist = {0};
	char				text_input[BUFF_MAX];
	Audio_wave			wave = {0};
	float				g_test = 1.0f;
	/* AudioData			g_play_sfx = {0}; */

Uint64				g_frequency;
Uint64				g_start;
Uint64				g_end;
Uint64				g_frame_count = 0;
Uint64				g_fps = 0;
double				g_elpsd = 0.0f;

void
postmix_callback(void *userdata, const SDL_AudioSpec *spec, float *buffer, 
		int buflen);

void 
init_sdl(void)
{
	/* frequency = SDL_GetPerformanceFrequency(); */
	QueryPerformanceFrequency(&wfreq);
	/* is set for the capture device sample in set_capture_device */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{ fprintf(stderr, "Init video|audio: %s\n", SDL_GetError()); exit(1); }

	if (TTF_Init() != 0)
	{ fprintf(stderr, "%s\n", SDL_GetError()); exit(1); }

	g_inst.window = SDL_CreateWindow("Key capture", g_win_w, g_win_h,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (g_inst.window == NULL)
	{ fprintf(stderr, "%s\n", SDL_GetError()); SDL_Quit(); exit(1); }

	g_inst.r = SDL_CreateRenderer(g_inst.window,NULL);
	if (g_inst.r == NULL)
		logExit("renderer failed to be created");

	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, true);
	char *font_path = "C:\\Users\\chiha\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Inconsolata-Regular.ttf";
	TTF_Font *ttf = TTF_OpenFont(font_path, 64);
	if (!ttf)
		logExit("Invalid font!\n");
	g_inst.w_form.mutex = SDL_CreateMutex();
	if (!g_inst.w_form.mutex)
		logExit("Mutex creation failed");
	
	g_inst.ttf = ttf;
}

Audio_wave
init_texture(void)
{
	Audio_wave wave = {.text = NULL, .w = g_win_w, .h = g_win_h/2, .current = 0};
	wave.text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, wave.w, wave.h);

	wave.rect = (SDL_FRect){.x = 0, .y = 0, .w = wave.w, .h = wave.h};
	SDL_SetRenderTarget(g_inst.r, wave.text);
	SDL_SetRenderDrawColor(g_inst.r, 50, 50, 50, 255);
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
	QueryPerformanceCounter(&wstart);
	TTF_SizeText(g_inst.ttf, msg, &w, &h);
	QueryPerformanceCounter(&wend);
	print_timer(wstart, wend, wfreq);
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
test(Uint8 *dst)
{
	SDL_LockMutex(g_inst.w_form.mutex);
	if (g_inst.w_form.open == false)
		goto end;
	YU_MixAudio(dst, (Uint8*)g_inst.w_form.buffer, SDL_AUDIO_F32,
			g_inst.w_form.buflen, g_test, &g_inst.w_form.wave);
	g_inst.w_form.open = false;
end:
	SDL_FRect view = {.x = 0, .y = g_win_h - wave.h, .w = wave.w, .h = wave.h};	
	SDL_SetRenderTarget(g_inst.r, NULL);
	SDL_RenderTexture(g_inst.r, wave.text, NULL, &view);
	SDL_UnlockMutex(g_inst.w_form.mutex);
}

/*
	  NOTE: dont draw in different thread! -> get the data use mutex and render in main
	  WARNING: adjust volume for file is deprected now
	  FIXME: couldnt add partial frames eof
	  FIXME: what happens when audio device changes/dies?
	  FIXME: stop function
	  BUG: Invalid file still on the list...
	  BUG: need double click to gain focus
	  BUG: SDL trusts blindly wav_header.. and crashes occur ! so remove LoadWav
	 * and use something else.
	 *
	 * note: logExit systematically quit, try to recover instead
	 * note: add timeline/scrubbing
	 * note: stream file / pull request to SDL?
	 * note: function to change police size (texture scale ?)
	 * note: add GUI slider
	 * note: add delete from playlist DEL key
	 * note: volume GUI
	 * note: add clickable text
	 * note: LIBAV ????????????????????????????????????????????? :D
	 * note: center ui buttons
	 * note: add wrapper timing functions os based -> see SDL_GetTick
	 * note: add focus when mouse above
	 *
	 * done: global buffer overflow resize big write font
	 * done: callback to change volume quicker
	 * done: cracklings sometimes in app.c;get_samples
	 * done: add drag&drop files to play 
 */
int
main(int ac, char **av)
{
			const char *cap_name = NULL;
			const char *out_name = NULL;
			if (ac >= 2) 
			{ 
				g_inst.capture_name = av[1]; cap_name = av[1];
				if (ac >=3)
				{ 
					g_inst.output_name = av[2]; out_name = av[2];
				}
			}
		init_sdl();
		SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
		LogicalDevice dev_cap = {0}; LogicalDevice dev_out = {0}; AudioData cap_data = {0};
		init_audio_device(&dev_cap, cap_name, CAPTURE, spec);
		init_audio_stream(&dev_cap, dev_cap.spec, CAPTURE);
		g_inst.capture_id = dev_cap.logical_id;
		g_inst.capture_dev = dev_cap;

			/* malloc c_data.buffer !! */
			/* care this doesnt work anymore !!!! PLAYLIST!!!*/
			/* cap_data = link_data_capture(dev_cap, dev_cap.stream, dev_cap.spec); */
			/* g_inst.stream = cap_data.stream; */

		/* should be done when saving */
		init_wav_header(&cap_data.header, cap_data.spec);
		init_audio_device(&dev_out, out_name, OUTPUT, g_playlist.music[0].spec);
		g_playlist.out_id = dev_out.logical_id;
		g_inst.out_id = dev_out.logical_id;
		g_inst.out_dev = dev_out;

		SDL_SetAudioStreamGetCallback(g_playlist.stream, put_callback, NULL);
		SDL_SetAudioPostmixCallback(g_inst.out_id, postmix_callback, NULL);
		g_inst.cursordefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
		if (!g_inst.cursordefault)
			logExit("Cursor failed");
		g_inst.cursorclick = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
		if (!g_inst.cursorclick)
			logExit("Cursor failed");
		SDL_SetCursor(g_inst.cursordefault);
		SDL_SetRenderDrawBlendMode(g_inst.r, SDL_BLENDMODE_BLEND);
		Camera2D cam = init_camera(0, 0, 1.0f);
		g_inst.cam = &cam;
		wave = init_texture();
		g_inst.nosongs.texture = create_static_text(g_inst.ttf, g_inst.r, "No songs");
		if (!g_inst.nosongs.texture)
			logExit("Could not get default 'No songs' texture");
		int w = 0, h = 0;
		if(TTF_SizeText(g_inst.ttf, "No songs", &w, &h) < 0)
			logExit("Could not get the size of the text");
		g_inst.nosongs.r.w = w;
		g_inst.nosongs.r.h = h;
		g_inst.nosongs.r.x = (int)(g_win_w/2) - 8;
		g_inst.nosongs.r.y = 100;

		init_button();
		memset(text_input, 0, BUFF_MAX);
		memset(g_playlist.music, 0, BUFF_MAX);
		g_inst.w_form.wave = wave;

	font f;
	init_font(&f, g_inst.r, g_inst.ttf);
	g_start = SDL_GetTicks();
	Uint8 dst[500000];
	memset(dst, 0, 500000);
	while (g_running)
	{
		Events(g_inst.e, &cap_data);
		set_new_frame(YU_GRAY);
        	/*
        	 * if (g_retrieving == 0)
			 *  	retrieve_stream_data(&c_data, c_data.stream, 1);
			 * if (g_sending == 0)
			 * {
			 * 	render_wave(&wave, c_data.buffer, c_data.header.dlength);
			 * 	g_sending = 1;
			 * }
        	 */
		draw_playlist(&f);
		test(dst);
		draw_buttons(g_inst.buttons);
		count_fps(&f);
		SDL_RenderPresent(g_inst.r);
		// Sleep(4); /* boring */
	}
	SDL_DestroyTexture(wave.text);
	/* too slow..  */
	/* SDL_free(sfx.buffer); */ 
	free(cap_data.buffer);
	free(g_inst.buttons);
	cleanup();
	return 0;
}

void
cleanup(void)
{
	/* SDL_DestroyTexture(g_inst.texture); */

	/* fucking slow these two */
	SDL_CloseAudioDevice(g_inst.out_id);
	SDL_CloseAudioDevice(g_inst.capture_id);
	/* SDL_DestroyMutex(g_playlist.mutex); */
	SDL_DestroyMutex(g_inst.w_form.mutex);

	SDL_DestroyCursor(g_inst.cursorclick);
	SDL_DestroyCursor(g_inst.cursordefault);
	SDL_DestroyAudioStream(g_playlist.stream);
	SDL_DestroyRenderer(g_inst.r);
	SDL_DestroyWindow(g_inst.window);
	TTF_Quit();
	SDL_Quit();
    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */
}
