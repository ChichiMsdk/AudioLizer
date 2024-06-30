#ifndef APP_H
#define APP_H

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_image.h>
#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "camera.h"
#include "gui.h"
#include "audio.h"

#define				MAX_BUFFER_SIZE 2000000000
#define				FIRST_ALLOC 1000 * 100
#define				BUFF_MAX 1024

#define				YU_GRAY (SDL_Color){ .r = 28, .g = 28, .b = 28, .a = 255}
#define				YU_WHITE (SDL_Color){255, 255, 255, 255}
#define				YU_RED (SDL_Color){195, 9, 24, 255}
#define				YU_BLUE_ATOLL (SDL_Color){0, 181, 215, 255}

typedef struct YUinstance YUinstance;

#ifdef				WIN_32
	#include			<windows.h>
	#include			<io.h>
	LARGE_INTEGER		wfreq;
	LARGE_INTEGER		wstart;
	LARGE_INTEGER		wend;
	float				welapsed;
#endif

	extern YUinstance		g_inst;
	extern int				g_win_w;
	extern int				g_win_h;
	extern int				g_retrieving;
	extern int				g_vizualizing;
	extern int				g_running;
	extern t_wav			g_wav_header;
	extern int				g_sending;
	extern int				g_BUFF_SIZE;
	extern int				g_playing;
	extern int				buff_end;
	extern double			g_volume;
	// extern AudioData		g_play_sfx;
	extern Playlist			g_playlist;
	extern char				text_input[BUFF_MAX];
	static unsigned int		g_nl;
	extern void				*g_buffer;
	extern float			g_test;
	extern Uint64			g_frequency;
	extern Uint64			g_start;
	extern Uint64			g_end;
	extern Uint64			g_frame_count;
	extern Uint64			g_fps;
	extern double			g_elpsd;
	// extern Audio_wave		wave;


typedef enum SKIP_OR_BACK
{
	SKIP = 0,
	BACK = 1
}SKIP_OR_BACK;

typedef struct poubelle
{
	SDL_Texture			*texture;
	SDL_FRect			r;
}poubelle;

typedef struct wave_form
{
	Audio_wave			wave;
	SDL_Mutex			*mutex;
	const SDL_AudioSpec	*spec;
	float				*buffer;
	size_t				buflen;
	bool				open;

}wave_form;

/* Check padding */
typedef struct YUinstance
{
	SDL_Window			*window;
	SDL_Renderer		*r;
	SDL_Rect			rect;
	SDL_Event			e;
	poubelle			nosongs;
	wave_form			w_form;

	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;
	LogicalDevice		capture_dev;
	LogicalDevice		out_dev;
	AudioData			sfx;
	TTF_Font			*ttf;

	FILE				*audio_file;
	char				*capture_name;
	char				*output_name;
	int					sample_size;
	size_t				current_buff_size;

	Camera2D			*cam;
	SDL_Cursor			*cursorclick;
	SDL_Cursor			*cursordefault;
	Button				button;
	Button				*buttons;
}YUinstance;

Mouse_state				get_mouse_state(void);
void					Events(SDL_Event e, AudioData *a_data);
void					cleanup(void);

// draw.c
void					set_new_frame(SDL_Color c);
Audio_wave				init_audio_wave(void);
SDL_Texture*			init_svg(char const *arr, int w, int h);
void					print_playlist(void);
void					draw_dynamic_text(font *f);
void					draw_text_music(SDL_FRect p, SDL_Color c, char const *msg, SDL_Texture *tex);
void					draw_text_texture(SDL_Point p, SDL_Color c, char const *msg, SDL_Texture *tex);
void					draw_playlist(font *f);
void					draw_wave_raw(Uint8 *dst);
void					count_fps(font *f);
Audio_wave				resize_texture(SDL_Texture *texture);

// app.c
void*					playlist_next(void *i);
void*					playlist_back(void *i);
void					print_playlist(void);
int						load_new_audio_to_play(const char *file_path, int desired, AudioData *a);
void					add_new_audio(const char *fname, int desired);
void					change_audio_to_play(int index, int desired);
void*					stop(void *i);
void*					replay(void *i);
void*					adjust_volume(float factor, void *buf, int length);
void*					my_toggle_play(void *sfx);
void					put_callback(void* usr, SDL_AudioStream *s, int add_amount, int total);
SDL_Texture*			init_svg(char const *arr, int w, int h);
int						get_samples(SDL_AudioSpec spec);
void					YU_MixAudio(Uint8 *dst, const Uint8 *src, SDL_AudioFormat format,
									 Uint32 len, float fvolume, Audio_wave *wave);

// log.c
#ifdef WIN_32
#include <windows.h>
void					print_timer(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq);
#endif
void					print_audio_spec_info(SDL_AudioSpec micSpec, int micSample);
void					logExit(char *msg);
void					print_stream_format();
void					debug_mouse_state(Mouse_state mouse);
								/* window renderer surface font */
void					logger(void *w, void *r, void *s, void *f, const char *msg);

//button.c
void					draw_button(Button button);
void					vizualize_stream_data(AudioData *audio_data, SDL_AudioStream *stream);
void					draw_buttons(Button *buttons);

#endif
