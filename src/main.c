#include			"app.h"
#include			"font.h"
#include			"ttf.h"

/* mandatory to launch app without console */
#define				SDL_MAIN_HANDLED
#include			<SDL3/SDL_main.h>

	YUinstance			g_inst = {0};
	int					g_win_w = 1200;
	int					g_win_h = 800;
	int					g_retrieving = 1;
	int					g_vizualizing = 1;
	int					g_running = 1;
	int					g_saving = 1;
	int					g_sending = 1;
	int					g_playing = 1;
	int					buff_end = 0;
	double				g_volume = 0.2f;
	void				*g_buffer = NULL;
	t_wav				g_wav_header = {0};
	Playlist			g_playlist = {0};
	char				text_input[BUFF_MAX];
	/* Audio_wave			wave = {0}; */
	float				g_test = 2.0f;
	Uint64				g_frequency;
	Uint64				g_start;
	Uint64				g_end;
	Uint64				g_frame_count = 0;
	Uint64				g_fps = 0;
	double				g_elpsd = 0.0f;
	font				g_f;
	/* AudioData			g_play_sfx = {0}; */

void
postmix_callback(void *userdata, const SDL_AudioSpec *spec, float *buffer, 
		int buflen);

void 
init_sdl(void)
{
	/* frequency = SDL_GetPerformanceFrequency(); */
#ifdef WIN_32
	QueryPerformanceFrequency(&wfreq);
#endif
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
	/* close this */
	SDL_IOStream *font_path = 
		SDL_IOFromConstMem(Inconsolata_Regular_ttf, Inconsolata_Regular_ttf_len);

	TTF_Font *ttf = TTF_OpenFontIO(font_path, SDL_TRUE, 48);
	if (!ttf)
		logExit("Invalid font!\n");
	g_inst.ttf = ttf;
	g_inst.w_form.mutex = SDL_CreateMutex();
	if (!g_inst.w_form.mutex)
		logExit("Mutex creation failed");
	g_inst.path_from = SDL_GetBasePath();
	if (!g_inst.path_from)
		logExit("Couldnt get the path from the app");
}

SDL_Texture*
resize_timeline_texture(SDL_Texture *texture)
{
	SDL_DestroyTexture(texture);
	SDL_Texture *text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, g_win_w, 10);

	SDL_SetRenderTarget(g_inst.r, text);
	YU_SetRenderDrawColor(g_inst.r, YU_GRAY);
	SDL_RenderClear(g_inst.r);
	return text;
}

SDL_Texture*
init_timeline_texture(void)
{
	SDL_Texture *text = SDL_CreateTexture(g_inst.r, SDL_PIXELFORMAT_UNKNOWN,
			SDL_TEXTUREACCESS_TARGET, g_win_w, 10);

	SDL_SetRenderTarget(g_inst.r, text);
	YU_SetRenderDrawColor(g_inst.r, YU_GRAY);
	SDL_RenderClear(g_inst.r);
	return text;
}

AudioData
trashcan(font *f, AudioData cap_data)
{
	const char *cap_name = NULL;
	const char *out_name = NULL;
	init_sdl();
	SDL_AudioSpec spec = {.freq = 44100, .format = SDL_AUDIO_S16LE, .channels = 1};
	LogicalDevice dev_cap = {0}; LogicalDevice dev_out = {0}; 
	init_audio_device(&dev_cap, cap_name, CAPTURE, spec);
	init_audio_stream(&dev_cap, dev_cap.spec, CAPTURE);
	g_inst.capture_id = dev_cap.logical_id;
	g_inst.capture_dev = dev_cap;

	/* malloc c_data.buffer !! */
	/* care this doesnt work anymore !!!! PLAYLIST!!!*/
    /*
	 * cap_data = link_data_capture(dev_cap, dev_cap.stream, dev_cap.spec);
	 * g_inst.stream = cap_data.stream;
     */
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
	init_font(f, g_inst.r, g_inst.ttf);
	g_start = SDL_GetTicks();
	g_inst.w_form.wave  = init_audio_wave();
	g_playlist.timeline_texture = init_timeline_texture();
	return cap_data;
}

void
format_time(char *str, int time)
{
    int minutes = time / 60;
    int seconds = time % 60;
    sprintf(str, "%02d:%02d", minutes, seconds);
}

void
write_music_time(int pos, int total)
{
	char str_time[100];
	char str_pos[6];
	char str_total[6];
	format_time(str_pos, pos);
	format_time(str_total, total);
	sprintf(str_time, "%s/%s", str_pos, str_total);
	int count = g_inst.buttons[0].count;
	SDL_FRect rect = g_inst.buttons[--count].rect;
	SDL_Point p = {.x = rect.x + rect.w + 30, .y = rect.y + 15};
	font_write(&g_f, g_inst.r, p, str_time);
	/* font_write(&g_f, g_inst.r, (SDL_Point){.x = g_win_w / 2, .y = g_win_h / 2}, str_time); */
}

void
draw_timeline(void)
{
	/* spec changed because of postmix callback function */
	if (!g_inst.w_form.spec)
		return ;
	/* BUG: crashes when changing device */
	int freq = g_inst.w_form.spec->freq;
	int format = SDL_AUDIO_BYTESIZE(g_inst.w_form.spec->format);
	int chan = g_inst.w_form.spec->channels;
	Uint32 sec = freq * format * chan;
	if (!sec)
		return;
	int pos = g_playlist.music[g_playlist.current].position / (freq * format * chan);
	int total = g_playlist.music[g_playlist.current].duration;
	float fpos = g_playlist.music[g_playlist.current].position / (float)(freq * format * chan);
	static float tmp;
	/* if (tmp != fpos) */
		/* printf("I entered %f\t tot: %d\n", fpos, total); */
	/* tmp = fpos; */

	write_music_time(fpos, total);
	float percent = (fpos / (float)total) * (float)g_win_w;
	SDL_FRect view = {.x = 0, .y = g_win_h - 35, .w = g_win_w, .h = 10};	

	SDL_FRect progress = {.x = 0, .y = 0, .w = percent, .h = 10};	
	SDL_SetRenderTarget(g_inst.r, g_playlist.timeline_texture);
	YU_SetRenderDrawColor(g_inst.r, YU_WHITE);
	SDL_RenderClear(g_inst.r);
	YU_SetRenderDrawColor(g_inst.r, YU_BLUE_ATOLL);
	SDL_RenderFillRect(g_inst.r, &progress);

	SDL_SetRenderTarget(g_inst.r, NULL);
	SDL_RenderTexture(g_inst.r, g_playlist.timeline_texture, NULL, &view);
	YU_SetRenderDrawColor(g_inst.r, YU_BLACK);
	DrawFilledCircle(g_inst.r, percent, view.y + 5, 13);
	YU_SetRenderDrawColor(g_inst.r, YU_WHITE);
	DrawFilledCircle(g_inst.r, percent, view.y + 5, 10);
}

/*
	  NOTE: dont draw in different thread! -> get the data use mutex and render in main
	  WARNING: adjust volume for file is deprected now
	  FIXME: weird bug with audio less than 3 sec it seems (or just audio?)
	  FIXME: couldnt add partial frames eof
	  FIXME: stop function
	  BUG: Invalid file still on the list...
	  BUG: need double click to gain focus
	  BUG: SDL trusts blindly wav_header.. and crashes occur ! so remove LoadWav
	 * and use something else.
	 *
	 * note: titles max size (gets trimmed)
	 * note: implements perceive loudness
	 * note: add timeline/scrubbing
	 * note: stream file / pull request to SDL?
	 * note: function to change police size (texture scale ?)
	 * note: add GUI slider
	 * note: add delete from playlist DEL key
	 * note: volume GUI
	 * note: add clickable text
	 * note: LIBAV ????????????????????????????????????????????? :D
	 * note: add wrapper timing functions os based -> see SDL_GetTick
	 * note: add focus when mouse above
	 * note: make SDL wrappers for better colors -> SDL_RenderDrawColor(color)
	 * note: logExit systematically quit, try to recover instead
	 *
	 * done: what happens when audio device changes/dies? seems ok
	 * done: center ui buttons
	 * done: add Pantone colors ISSOU
	 * done: couldnt add partial frames eof
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
	Camera2D cam = init_camera(0, 0, 1.0f);
	g_inst.cam = &cam;
	AudioData cap_data = {0};
	cap_data = trashcan(&g_f, cap_data);
	Uint8 dst[50000];
	Uint8 *tt;
	memset(dst, 0, 50000);
	TracyCZoneCtx a;
	printf("yo\n");
	while (g_running)
	{
        	/*
        	 * if (g_retrieving == 0)
			 *  	retrieve_stream_data(&c_data, c_data.stream, 1);
			 * if (g_sending == 0)
			 * {
			 * 	render_wave(&wave, c_data.buffer, c_data.header.dlength);
			 * 	g_sending = 1;
			 * }
        	 */
		Events(g_inst.e, &cap_data);
		set_new_frame(YU_GRAY);
		TracyCZoneN(a, "wave", 1)
		draw_wave_raw(dst);
		TracyCZoneEnd(a);
		draw_playlist(&g_f);
		draw_timeline();
		draw_buttons(g_inst.buttons);
		/* count_fps(&g_f); */
		SDL_RenderPresent(g_inst.r);
#ifdef WIN_32
		Sleep(10); /* boring */
#endif
		TracyCFrameMark; 
	}
	SDL_DestroyTexture(g_inst.w_form.wave.text);
	/* too slow..  */
	SDL_free(g_playlist.music[g_playlist.current].buffer);
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
	SDL_free(g_inst.path_from);
	SDL_CloseAudioDevice(g_inst.out_id);
	SDL_CloseAudioDevice(g_inst.capture_id);
	/* SDL_DestroyMutex(g_playlist.mutex); */
	SDL_DestroyMutex(g_inst.w_form.mutex);

	SDL_DestroyCursor(g_inst.cursorclick);
	SDL_DestroyCursor(g_inst.cursordefault);
	SDL_DestroyAudioStream(g_playlist.stream);
	TTF_CloseFont(g_inst.ttf);
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
