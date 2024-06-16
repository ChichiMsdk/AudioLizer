#include "app.h"

int					g_playing;
float				g_volume;
AudioData			*g_sfx;

void
my_toggle_play(AudioData *sfx);

void
key_down(SDL_Keycode key)
{
	/* record_pressed(key); */
	switch (key)
	{
		case SDLK_ESCAPE:
			g_running = 0;
			break;
		case SDLK_UP:
			g_volume *= 1.1f;
			if (g_volume == 0)
				g_volume = 0.1;
			if (g_volume >= 2.5)
				g_volume = 2.5;
			printf("Volume = %f\n", g_volume);
			break;
		case SDLK_DOWN:
			g_volume /= 1.1f;
			if (g_volume <= 0.1)
				g_volume = 0.0;
			printf("Volume = %f\n", g_volume);
			break;
		case SDLK_LEFT:
			break;
		case SDLK_RIGHT:
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
			my_toggle_play(g_sfx);
			g_sending = !g_sending;
            /*
			 * g_BUFF_SIZE++;
			 * if (g_BUFF_SIZE > 32 )
			 * 	g_BUFF_SIZE = 1;
             */
			break;
		case SDLK_s:
			/* should be prompted to change the name of the file */
			save_file(filename, a_data);
			break;
		case SDLK_r:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Paused recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
					SDL_FlushAudioStream(g_inst.stream);
					g_retrieving = 1;
				}
				else
				{
					printf("Resume recording!\n");
					SDL_ResumeAudioDevice(g_inst.capture_id);
					g_retrieving = 0;
				}
				break;
			}
		case SDLK_f:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Stopped recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
				}
				printf("Clearing the stream!\n");
				SDL_ClearAudioStream(g_inst.stream);
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
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					button_check_released(get_mouse_state(), g_inst.buttons);
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
					key_down(e.key.keysym.sym);
					break;
				}
			case SDL_EVENT_KEY_UP:
				{
					key_up(e.key.keysym.sym, a_data);
					break;
				}
			case SDL_EVENT_MOUSE_WHEEL:
				return;
				mouse_wheel(e.wheel);
				break;
		}
	}
}
