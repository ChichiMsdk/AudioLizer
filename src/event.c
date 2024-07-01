#include "app.h"

#define LINE_SIZE_MAX 1024

int
isText(Uint32 key)
{
	if (key < 127 && key >= 32)
		return true;
	return false;
}

void 
isTyping(Uint32 event_key)
{
	static Uint32 key;
	key = event_key;
	if (key == SDLK_RETURN)
	{
		text_input[buff_end] = '\n';
		buff_end++;
		g_nl++;
		/* clearLine(); */
		/* yu_print(text_input); */
		assert(buff_end < LINE_SIZE_MAX);
		return ;
	}
	if (isText(key) == false)
	{
		return ;
	}
	if (key == SDLK_RETURN)
		text_input[buff_end] = '\n';
	else
		text_input[buff_end] = key;
	buff_end++;
	/* clearLine(); */
	/* yu_print(text_input); */
	assert(buff_end < LINE_SIZE_MAX);
}

void
isDeleting(void)
{
	if (buff_end > 0)
	{
		// deletes last character entry
		buff_end--;
		if (text_input[buff_end] == '\n')
		{
			g_nl--;
		}
		text_input[buff_end] = 0;
	}
	/* clearLine(); */
	/* yu_print(text_input); */
}

void
key_down(SDL_Keycode key)
{
	/* record_pressed(key); */
	switch (key)
	{
		case SDLK_ESCAPE:
			g_running = 0;
			break;
		case SDLK_BACKSPACE:
			break;
		case SDLK_KP_PLUS:
			g_volume *= 1.1f;
			if (g_volume == 0)
				g_volume = 0.0000002;
			if (g_volume >= 2.5)
				g_volume = 2.5;
			printf("Volume = %f\n", g_volume);
			break;
		case SDLK_KP_MINUS:
			g_volume /= 1.1f;
			if (g_volume <= 0.0000001)
				g_volume = 0.0000001;
			printf("Volume = %f\n", g_volume);
			break;
		case SDLK_LEFT:
			printf("factor %f\n", g_test);
			g_test/=2;
			if (g_test <= 0)
				g_test = 1;
			break;
		case SDLK_RIGHT:
			printf("factor %f\n", g_test);
			g_test*=2;
			if (g_test > 10000000)
				g_test = 10000000;
			break;
		case SDLK_y:
			g_test = 1;
			break;
		case SDLK_DOWN:
			playlist_next(NULL);
			break;
		case SDLK_UP:
			playlist_back(NULL);
			break;
	}
}

void
key_up(SDL_Keycode key, AudioData *a_data)
{
	char *filename = "audio2.wav";
	/* record_released(key); */
	switch (key)
	{
		case SDLK_l:
			my_toggle_play(&g_playlist);
			g_sending = !g_sending;
            /*
			 * g_BUFF_SIZE++;
			 * if (g_BUFF_SIZE > 32 )
			 * 	g_BUFF_SIZE = 1;
             */
			break;
		case SDLK_s:
			/* should be prompted to change the name of the file */
			/* save_file(filename, a_data); */
			break;
		case SDLK_r:
			{
				print_playlist();
                /*
				 * if (!SDL_AudioDevicePaused(g_inst.capture_id))
				 * {
				 * 	printf("Paused recording..\n");
				 * 	SDL_PauseAudioDevice(g_inst.capture_id);
				 * 	SDL_FlushAudioStream(g_inst.stream);
				 * 	g_retrieving = 1;
				 * }
				 * else
				 * {
				 * 	printf("Resume recording!\n");
				 * 	SDL_ResumeAudioDevice(g_inst.capture_id);
				 * 	g_retrieving = 0;
				 * }
                 */
				break;
			}
		case SDLK_f:
			{
                /*
				 * if (!SDL_AudioDevicePaused(g_inst.capture_id))
				 * {
				 * 	printf("Stopped recording..\n");
				 * 	SDL_PauseAudioDevice(g_inst.capture_id);
				 * }
				 * printf("Clearing the stream!\n");
				 * SDL_ClearAudioStream(g_inst.stream);
                 */
				break;
			}
	}
}

void
mouse_wheel(SDL_MouseWheelEvent wheel)
{
	float x, y;
	SDL_GetMouseState(&x, &y);
	/* printf("wheel.y = %f\n", wheel.y); */
	if (wheel.y > 0) 
	{
		zoom_camera(g_inst.cam, 1.1f, x, y); // Zoom in
	} 
	else if (wheel.y < 0)
	{
		zoom_camera(g_inst.cam, 0.9f, x, y); // Zoom in
	}
}

void
drop_event(char *fname)
{
	/* printf("len_file_name: %llu\n", strlen(fname)); */
	AudioData audio = {0};
	if (load_new_audio_to_play(fname, 0, &audio) < 0)
	{
		/* replays the previous one */
		change_audio_to_play(g_playlist.current, 0);
		return ;
	}
	g_playlist.music[g_playlist.current] = audio;
	/* SDL_RaiseWindow(g_inst.window); */
	/* SDL_SetAudioStreamGetCallback(g_play_sfx.stream, put_callback, &g_play_sfx); */
}

SDL_Texture*
resize_timeline_texture(SDL_Texture *texture);

void
Events(SDL_Event e, AudioData *a_data)
{
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
			case SDL_EVENT_QUIT:
				{
					g_running = 0;
					break;
				}
			case SDL_EVENT_DROP_FILE:
				{
					printf("%s\n", e.drop.data);
					drop_event(e.drop.data);
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					button_check_released(get_mouse_state(), g_inst.buttons);
					/* debug_mouse_state(get_mouse_state()); */
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					button_check_pressed(get_mouse_state(), g_inst.buttons);
					/* debug_mouse_state(get_mouse_state()); */
					break;
				}
			case SDL_EVENT_MOUSE_MOTION:
				{
					button_check_hover(get_mouse_state(), g_inst.buttons);
					break;
				}
			case SDL_EVENT_KEY_DOWN:
				{
					/* should I break here ? */
					key_down(e.key.key);
					break;
				}
			case SDL_EVENT_KEY_UP:
				{
					key_up(e.key.key, a_data);
					break;
				}
			case SDL_EVENT_MOUSE_WHEEL:
				return;
				mouse_wheel(e.wheel);
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				{
					SDL_LockMutex(g_inst.w_form.mutex);
					SDL_GetWindowSize(g_inst.window, &g_win_w, &g_win_h);
					g_inst.w_form.wave = resize_texture(g_inst.w_form.wave.text);
					g_playlist.timeline_texture = resize_timeline_texture(g_playlist.timeline_texture);
					SDL_UnlockMutex(g_inst.w_form.mutex);
					break;
				}
		}
	}
}
