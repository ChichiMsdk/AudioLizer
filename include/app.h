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

typedef struct YUinstance YUinstance;

	extern YUinstance		g_inst;
	extern int				WINDOW_WIDTH;
	extern int				WINDOW_HEIGHT;
	extern int				g_retrieving;
	extern int				g_vizualizing;
	extern int				g_running;
	extern t_wav			g_wav_header;
	extern int				g_sending;
	extern int				g_BUFF_SIZE;
	extern int				g_playing;
	extern int				buff_end;
	extern float			g_volume;
	// extern AudioData		g_play_sfx;
	extern Playlist			g_playlist;
	extern char				text_input[BUFF_MAX];
	static unsigned int		g_nl;
	extern void				*g_buffer;


typedef enum SKIP_OR_BACK
{
	SKIP = 0,
	BACK = 1
}SKIP_OR_BACK;

/* Check padding */
typedef struct YUinstance
{
	SDL_Window			*window;
	SDL_Renderer		*r;
	SDL_Rect			rect;
	SDL_Event			e;
	SDL_Texture			*texture;

	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;
	AudioData			sfx;

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

// app.c

void*					playlist_next(void *i);
void*					playlist_back(void *i);
void					print_playlist(void);
AudioData				load_new_audio_to_play(const char *fname, int desired);
void					add_new_audio(const char *fname, int desired);
void					change_audio_to_play(int index, int desired);
void*					stop(void *i);
void*					replay(void *i);
uint8_t*				adjust_volume(float factor, uint8_t *buf, int length);
void*					my_toggle_play(void *sfx);
void					put_callback(void* usr, SDL_AudioStream *s, int add_amount, int total);
SDL_Texture*			init_svg(char const *arr, int w, int h);

// log.c
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
